#include <strobe/flow/program.hpp>

#include "strobe/flow/detail/program_runtime.hpp"

namespace strobe::flow {

Program::Program() = default;

Program::Program(detail::ProgramData data) : m_data(std::move(data)) {
  if (m_data.activation_layout.size == 0 && !m_data.nodes.empty()) {
    auto &layout = m_data.activation_layout;
    layout.capacity = 1;
    std::size_t offset = 0;
    const auto append = [&](std::size_t count, std::size_t size,
                            std::size_t alignment) {
      offset = (offset + alignment - 1) & ~(alignment - 1);
      const auto result = offset;
      offset += count * size;
      layout.alignment = std::max(layout.alignment, alignment);
      return result;
    };
    layout.values_offset =
        append(m_data.values.size(), sizeof(void *), alignof(void *));
    layout.arguments_offset =
        append(m_data.accesses.size(), sizeof(void *), alignof(void *));
    layout.generations_offset =
        append(m_data.resource_slots.size(),
               sizeof(std::optional<detail::ResourceGeneration>),
               alignof(std::optional<detail::ResourceGeneration>));
    layout.previous_generations_offset =
        append(m_data.resource_slots.size(),
               sizeof(std::optional<detail::ResourceGeneration>),
               alignof(std::optional<detail::ResourceGeneration>));
    layout.changed_values_offset = append(
        m_data.values.size(), sizeof(std::uint8_t), alignof(std::uint8_t));
    offset = std::max(offset, std::size_t{1});
    layout.stride = (offset + layout.alignment - 1) & ~(layout.alignment - 1);
    layout.size = layout.stride;
  }
  m_runtime = make_box<detail::ProgramRuntime>(allocator_ref{}, m_data);
}

Program::Program(Program &&) noexcept = default;

Program &Program::operator=(Program &&) noexcept = default;

Program::~Program() = default;

const detail::ProgramData &Program::data() const noexcept { return m_data; }

detail::ProgramData &Program::data() noexcept { return m_data; }

std::size_t Program::operation_count() const noexcept {
  return m_data.source_operation_count;
}

std::size_t Program::resource_count() const noexcept {
  return m_data.source_resource_count;
}

std::size_t Program::external_count() const noexcept {
  return m_data.source_external_count;
}

void Program::bind_external(std::string_view name, void *pointer) {
  runtime().bind(m_data, name, pointer);
}

void *Program::external(ResourceId resource) const {
  return runtime().external(m_data, resource);
}

bool Program::externals_bound() const {
  return runtime().all_externals_bound();
}

bool Program::libraries_bound() const noexcept {
  return std::all_of(m_data.callbacks.begin(), m_data.callbacks.end(),
                     [](const detail::CompiledCallback &callback) {
                       return !callback.external ||
                              static_cast<bool>(callback.runtime);
                     });
}

void Program::bind_library(std::string_view library_id, Library library) {
  if (m_scheduler != nullptr) {
    throw std::logic_error(
        "Flow libraries must be bound before the first submission");
  }
  if (library_id.empty() || library.resolve == nullptr) {
    throw std::invalid_argument("Flow library binding is invalid");
  }
  Vector<std::size_t, allocator_ref> callback_indices;
  Vector<ExternalOpFunction, allocator_ref> functions;
  for (std::size_t index = 0; index < m_data.callbacks.size(); ++index) {
    const auto &callback = m_data.callbacks[index];
    if (!callback.external ||
        callback.external->library_id.view() != library_id) {
      continue;
    }
    if (callback.argument_types.size() != callback.argument_accesses.size()) {
      throw std::logic_error(
          "Flow external op argument metadata is inconsistent");
    }
    const auto function =
        library.resolve(library.state, callback.external->symbol.view(),
                        callback.external->abi_version);
    if (function == nullptr) {
      throw std::invalid_argument("Flow external op symbol was not resolved");
    }
    callback_indices.push_back(index);
    functions.push_back(function);
  }
  if (callback_indices.empty()) {
    throw std::invalid_argument("Flow library id is not declared");
  }
  for (std::size_t resolved = 0; resolved < callback_indices.size();
       ++resolved) {
    auto &callback = m_data.callbacks[callback_indices[resolved]];
    const auto function = functions[resolved];
    const auto abi_version = callback.external->abi_version;
    auto argument_types = callback.argument_types;
    auto argument_accesses = callback.argument_accesses;
    callback.runtime =
        [function, abi_version, argument_types = std::move(argument_types),
         argument_accesses = std::move(argument_accesses)](
            Lifetime lifetime, span<void *const> arguments) noexcept {
          const ExternalOpContext context{
              .abi_version = abi_version,
              .lifetime = lifetime,
              .argument_count = static_cast<std::uint32_t>(arguments.size()),
              .arguments = arguments.data(),
              .argument_types = argument_types.data(),
              .argument_accesses = argument_accesses.data(),
          };
          function(context);
        };
  }
}

void Program::release(std::string_view name, std::uint64_t count) {
  runtime().release(m_data, name, count);
}

std::optional<detail::ResourceGeneration>
Program::try_acquire_generation(ResourceId resource) {
  return runtime().try_acquire(resource);
}

detail::ResourceGeneration Program::acquire_generation(ResourceId resource) {
  return runtime().acquire(resource);
}

void *Program::resolve(const detail::ResourceGeneration &generation) const {
  return runtime().resolve(generation);
}

void Program::mark_constructed(const detail::ResourceGeneration &generation) {
  runtime().mark_constructed(generation);
}

void Program::release_generation(const detail::ResourceGeneration &generation) {
  runtime().release(generation);
}

void Program::rethrow_execution_error() const { runtime().rethrow_error(); }

sync::job_id Program::submit(sync::JobScheduler *scheduler) {
  struct NodeJob {
    Program *program;
    std::uint32_t activation;
    detail::NodeId node;

    void operator()() noexcept {
      auto &runtime = program->runtime();
      if (runtime.failed()) {
        return;
      }
      try {
        program->execute_node(activation, node, false);
      } catch (...) {
        runtime.record_error(std::current_exception());
      }
    }
  };

  struct ControlJob {
    Program *program;
    std::uint32_t activation;
    detail::ControlId control;

    std::uint32_t operator()(std::uint32_t invocation) noexcept {
      auto &runtime = program->runtime();
      if (runtime.failed()) {
        return 0;
      }
      try {
        return program->execute_control_iteration(control, activation,
                                                  invocation)
                   ? 1U
                   : 0U;
      } catch (...) {
        runtime.record_error(std::current_exception());
        return 0;
      }
    }
  };

  struct ReleaseJob {
    Program *program;
    std::uint32_t activation;
    ResourceId resource;

    void operator()() noexcept {
      auto &runtime = program->runtime();
      try {
        runtime.release_active(program->m_data, activation, resource);
      } catch (...) {
        runtime.record_error(std::current_exception());
      }
    }
  };

  struct FinishJob {
    Program *program;
    std::uint32_t activation;
    bool activation_complete;

    void operator()() noexcept {
      auto &runtime = program->runtime();
      try {
        runtime.finish_execution(program->m_data, activation,
                                 activation_complete);
      } catch (...) {
        runtime.record_error(std::current_exception());
      }
    }
  };

  struct EmptyJob {
    void operator()() noexcept {}
  };

  static_assert(sync::job_fn<NodeJob>);
  static_assert(sync::job_fn<ControlJob>);
  static_assert(sync::job_fn<ReleaseJob>);
  static_assert(sync::job_fn<FinishJob>);
  static_assert(sync::job_fn<EmptyJob>);

  if (scheduler == nullptr) {
    throw std::invalid_argument("Flow scheduler cannot be null");
  }
  if (m_scheduler == nullptr) {
    m_scheduler = scheduler;
  } else if (m_scheduler != scheduler) {
    throw std::invalid_argument(
        "Flow program must use the scheduler from its first submission");
  }
  runtime().clear_error();
  for (detail::NodeId node = 0; node < m_data.nodes.size(); ++node) {
    const auto &compiled = m_data.nodes[node];
    if (compiled.callback != detail::null_callback_id &&
        compiled.callback >= m_data.callbacks.size()) {
      throw std::logic_error("compiled node callback is out of range");
    }
    if (compiled.callback != detail::null_callback_id) {
      const auto &callback = m_data.callbacks[compiled.callback];
      if (!callback.runtime) {
        throw std::logic_error("not all Flow external op libraries are bound");
      }
    }
    if (compiled.predecessors.offset + compiled.predecessors.count >
        m_data.predecessors.size()) {
      throw std::logic_error("compiled node predecessors are out of range");
    }
    for (std::uint32_t index = 0; index < compiled.predecessors.count;
         ++index) {
      if (m_data.predecessors[compiled.predecessors.offset + index] >= node) {
        throw std::logic_error(
            "compiled jobs must be stored in dependency order");
      }
    }
    if (compiled.accesses.offset + compiled.accesses.count >
        m_data.accesses.size()) {
      throw std::logic_error("compiled node accesses are out of range");
    }
  }

  std::optional<sync::job_id> aggregate;
  Vector<sync::job_id, allocator_ref> previous_activation_jobs;
  Vector<bool, allocator_ref> previous_activation_job_valid;
  while (true) {
    const auto first_node = m_resume_node;
    auto end_node = static_cast<detail::NodeId>(m_data.nodes.size());
    bool hit_yield = false;
    for (detail::NodeId node = first_node; node < end_node; ++node) {
      if (m_data.nodes[node].kind == detail::OperationKind::yield) {
        end_node = node + 1;
        hit_yield = true;
        break;
      }
      if (m_data.nodes[node].kind != detail::OperationKind::acquire) {
        continue;
      }
      const detail::CompiledAcquire *compiled_acquire = nullptr;
      for (const auto &acquire : m_data.acquires) {
        if (acquire.node == node) {
          compiled_acquire = &acquire;
          break;
        }
      }
      if (compiled_acquire == nullptr) {
        throw std::logic_error("acquire node has no runtime descriptor");
      }
      if (!runtime().try_consume_token(compiled_acquire->semaphore)) {
        end_node = node;
        break;
      }
    }
    const bool activation_complete = end_node == m_data.nodes.size();
    if (end_node == first_node) {
      break;
    }

    std::uint32_t activation = 0;
    Vector<sync::job_id, allocator_ref> jobs;
    Vector<bool, allocator_ref> job_valid;
    jobs.resize(m_data.nodes.size());
    job_valid.resize(m_data.nodes.size(), false);
    if (first_node == 0) {
      activation = runtime().begin_execution(m_data);
    } else {
      if (!m_resume_activation) {
        throw std::logic_error("suspended program has no activation state");
      }
      activation = *m_resume_activation;
      runtime().resume_execution(activation);
    }
    const bool has_previous_activation =
        first_node == 0 &&
        previous_activation_jobs.size() == m_data.nodes.size();

    Vector<sync::job_id, allocator_ref> releases;
    releases.reserve(m_data.resource_lifetimes.size());
    Vector<bool, allocator_ref> control_submitted(m_data.controls.size(), false,
                                                  allocator_ref{});
    for (detail::NodeId node = first_node; node < end_node; ++node) {
      const auto control = node < m_data.control_of_node.size()
                               ? m_data.control_of_node[node]
                               : detail::null_control_id;
      if (control != detail::null_control_id) {
        if (control >= m_data.controls.size()) {
          throw std::logic_error("structured control id is out of range");
        }
        if (control_submitted[control]) {
          continue;
        }
        const auto &compiled_control = m_data.controls[control];
        if (compiled_control.parent != detail::null_control_id ||
            compiled_control.nodes.offset + compiled_control.nodes.count >
                m_data.control_nodes.size()) {
          throw std::logic_error("top-level structured control is invalid");
        }
        const auto job = scheduler->acquire();
        Vector<sync::job_id, allocator_ref> dependencies;
        const auto add_dependency = [&](sync::job_id dependency) {
          if (std::find(dependencies.begin(), dependencies.end(), dependency) ==
              dependencies.end()) {
            dependencies.push_back(dependency);
            scheduler->addDependency(job, dependency);
          }
        };
        for (std::uint32_t member_index = 0;
             member_index < compiled_control.nodes.count; ++member_index) {
          const auto member =
              m_data
                  .control_nodes[compiled_control.nodes.offset + member_index];
          if (member < first_node || member >= end_node) {
            throw std::logic_error(
                "structured control crosses a submission boundary");
          }
          const auto &member_node = m_data.nodes[member];
          for (std::uint32_t index = 0; index < member_node.predecessors.count;
               ++index) {
            const auto predecessor =
                m_data.predecessors[member_node.predecessors.offset + index];
            const auto predecessor_control =
                predecessor < m_data.control_of_node.size()
                    ? m_data.control_of_node[predecessor]
                    : detail::null_control_id;
            if (predecessor >= first_node && predecessor_control != control) {
              if (!job_valid[predecessor]) {
                throw std::logic_error(
                    "structured control predecessor was not submitted");
              }
              add_dependency(jobs[predecessor]);
            }
          }
          if (has_previous_activation) {
            if (previous_activation_job_valid[member]) {
              add_dependency(previous_activation_jobs[member]);
            }
            for (std::uint32_t index = 0;
                 index < member_node.temporal_predecessors.count; ++index) {
              const auto predecessor =
                  m_data.temporal_predecessors
                      [member_node.temporal_predecessors.offset + index];
              const auto predecessor_control =
                  predecessor < m_data.control_of_node.size()
                      ? m_data.control_of_node[predecessor]
                      : detail::null_control_id;
              if (predecessor_control != control &&
                  previous_activation_job_valid[predecessor]) {
                add_dependency(previous_activation_jobs[predecessor]);
              }
            }
          }
        }
        scheduler->submit(job, ControlJob{this, activation, control});
        control_submitted[control] = true;
        for (std::uint32_t member_index = 0;
             member_index < compiled_control.nodes.count; ++member_index) {
          const auto member =
              m_data
                  .control_nodes[compiled_control.nodes.offset + member_index];
          jobs[member] = job;
          job_valid[member] = true;
        }
        continue;
      }
      const auto &compiled = m_data.nodes[node];
      if (compiled.kind == detail::OperationKind::construct &&
          !compiled.memoized &&
          !m_data
               .resource_lifetimes[m_data.accesses[compiled.accesses.offset]
                                       .resource]
               .preconstructed &&
          runtime().will_execute(m_data, activation, node)) {
        if (compiled.accesses.count == 0) {
          throw std::logic_error("construction has no compiled output");
        }
        runtime().acquire_for_execution(
            m_data, activation,
            m_data.accesses[compiled.accesses.offset].resource);
      } else if (compiled.kind == detail::OperationKind::discard &&
                 compiled.accesses.count != 0 &&
                 m_data
                     .resource_lifetimes
                         [m_data.accesses[compiled.accesses.offset].resource]
                     .preconstructed) {
        runtime().acquire_for_execution(
            m_data, activation,
            m_data.accesses[compiled.accesses.offset].resource);
      } else if (compiled.kind == detail::OperationKind::phi) {
        bool found = false;
        for (const auto &phi : m_data.phis) {
          if (phi.node == node) {
            runtime().select_phi(m_data, activation, phi);
            found = true;
            break;
          }
        }
        if (!found) {
          throw std::logic_error("phi node has no runtime descriptor");
        }
      }

      const auto job = scheduler->acquire();
      Vector<sync::job_id, allocator_ref> dependencies;
      const auto add_dependency = [&](sync::job_id dependency) {
        if (std::find(dependencies.begin(), dependencies.end(), dependency) ==
            dependencies.end()) {
          dependencies.push_back(dependency);
          scheduler->addDependency(job, dependency);
        }
      };
      for (std::uint32_t index = 0; index < compiled.predecessors.count;
           ++index) {
        const auto predecessor =
            m_data.predecessors[compiled.predecessors.offset + index];
        if (predecessor >= first_node) {
          if (!job_valid[predecessor]) {
            throw std::logic_error("node predecessor was not submitted");
          }
          add_dependency(jobs[predecessor]);
        }
      }
      if (has_previous_activation) {
        for (std::uint32_t index = 0;
             index < compiled.temporal_predecessors.count; ++index) {
          const auto predecessor =
              m_data
                  .temporal_predecessors[compiled.temporal_predecessors.offset +
                                         index];
          if (previous_activation_job_valid[predecessor]) {
            add_dependency(previous_activation_jobs[predecessor]);
          }
        }
        if ((compiled.memoized || compiled.runs_once) &&
            previous_activation_job_valid[node]) {
          add_dependency(previous_activation_jobs[node]);
        }
        for (const auto &control_descriptor : m_data.controls) {
          if (control_descriptor.parent != detail::null_control_id ||
              control_descriptor.initial_condition_node != node) {
            continue;
          }
          if (control_descriptor.nodes.offset + control_descriptor.nodes.count >
                  m_data.control_nodes.size() ||
              control_descriptor.nodes.count == 0) {
            throw std::logic_error("structured control nodes are invalid");
          }
          const auto member =
              m_data.control_nodes[control_descriptor.nodes.offset];
          if (previous_activation_job_valid[member]) {
            add_dependency(previous_activation_jobs[member]);
          }
        }
      }
      scheduler->submit(job, NodeJob{this, activation, node});
      jobs[node] = job;
      job_valid[node] = true;

      for (ResourceId resource = 0; resource < m_data.resource_lifetimes.size();
           ++resource) {
        const auto &lifetime = m_data.resource_lifetimes[resource];
        const bool release_preconstructed =
            lifetime.preconstructed && !lifetime.retained_by_feedback;
        if ((lifetime.destroy_at_shutdown && !release_preconstructed) ||
            lifetime.release_after.count == 0 ||
            lifetime.release_after.offset + lifetime.release_after.count >
                m_data.release_after.size()) {
          continue;
        }
        detail::NodeId last_frontier = 0;
        for (std::uint32_t index = 0; index < lifetime.release_after.count;
             ++index) {
          last_frontier = std::max(
              last_frontier,
              m_data.release_after[lifetime.release_after.offset + index]);
        }
        if (last_frontier != node) {
          continue;
        }
        const auto release = scheduler->acquire();
        for (std::uint32_t index = 0; index < lifetime.release_after.count;
             ++index) {
          const auto frontier =
              m_data.release_after[lifetime.release_after.offset + index];
          if (frontier >= end_node) {
            throw std::logic_error("release frontier is out of range");
          }
          if (frontier >= first_node) {
            scheduler->addDependency(release, jobs[frontier]);
          }
        }
        scheduler->submit(release, ReleaseJob{this, activation, resource});
        releases.push_back(release);
      }
    }

    const auto completion = scheduler->acquire();
    Vector<sync::job_id, allocator_ref> completion_dependencies;
    const auto add_completion_dependency = [&](sync::job_id dependency) {
      if (std::find(completion_dependencies.begin(),
                    completion_dependencies.end(),
                    dependency) == completion_dependencies.end()) {
        completion_dependencies.push_back(dependency);
        scheduler->addDependency(completion, dependency);
      }
    };
    for (detail::NodeId node = first_node; node < end_node; ++node) {
      bool has_successor_in_segment = false;
      const auto &compiled = m_data.nodes[node];
      for (std::uint32_t index = 0; index < compiled.successors.count;
           ++index) {
        const auto successor =
            m_data.successors[compiled.successors.offset + index];
        has_successor_in_segment |=
            successor >= first_node && successor < end_node;
      }
      if (!has_successor_in_segment) {
        if (!job_valid[node]) {
          throw std::logic_error("completion node was not submitted");
        }
        add_completion_dependency(jobs[node]);
      }
    }
    for (const auto release : releases) {
      add_completion_dependency(release);
    }
    scheduler->submit(completion,
                      FinishJob{this, activation, activation_complete});

    const auto next_aggregate = scheduler->acquire();
    scheduler->addDependency(next_aggregate, completion);
    if (aggregate) {
      scheduler->addDependency(next_aggregate, *aggregate);
    }
    scheduler->submit(next_aggregate, EmptyJob{});
    aggregate = next_aggregate;

    if (activation_complete) {
      previous_activation_jobs = jobs;
      previous_activation_job_valid = job_valid;
      m_resume_node = 0;
      m_resume_activation = std::nullopt;
    } else {
      m_resume_node = end_node;
      m_resume_activation = activation;
    }

    if (hit_yield || !activation_complete || m_data.acquires.empty()) {
      break;
    }
  }

  if (!aggregate) {
    if (!m_last_completion) {
      m_last_completion = scheduler->submit(EmptyJob{});
    }
    return *m_last_completion;
  }
  m_last_completion = aggregate;
  return *m_last_completion;
}

void Program::execute_node(std::uint32_t activation, detail::NodeId node,
                           bool prepare, bool force) {
  if (node >= m_data.nodes.size()) {
    throw std::logic_error("compiled node is out of range");
  }
  auto &compiled = m_data.nodes[node];
  auto &state = runtime();
  if (!force && !state.should_execute(m_data, activation, node)) {
    if (compiled.memoized) {
      state.retain_outputs(m_data, activation, compiled);
    }
    return;
  }
  if (compiled.kind == detail::OperationKind::cutoff) {
    for (std::size_t index = 0; index < m_data.cutoffs.size(); ++index) {
      if (m_data.cutoffs[index].node == node) {
        state.evaluate_cutoff(m_data, activation, index);
        state.complete_node(m_data, node);
        return;
      }
    }
    throw std::logic_error("cutoff node has no runtime descriptor");
  }
  if (compiled.kind == detail::OperationKind::construct &&
      compiled.accesses.count != 0) {
    const auto resource = m_data.accesses[compiled.accesses.offset].resource;
    if (resource < m_data.resource_lifetimes.size() &&
        m_data.resource_lifetimes[resource].preconstructed) {
      if (compiled.callback == detail::null_callback_id ||
          compiled.callback >= m_data.callbacks.size()) {
        throw std::logic_error(
            "preconstructed resource has no construction callback");
      }
      auto &callback = m_data.callbacks[compiled.callback];
      const auto slots = m_data.resource_slots[resource].slot_count;
      for (std::uint32_t slot = 0; slot < slots; ++slot) {
        state.acquire_for_preconstruction(m_data, activation, resource);
        bool acquired = true;
        try {
          state.prepare_arguments(m_data, activation, compiled);
          callback.runtime(m_data.resource_lifetimes[resource].interval,
                           state.arguments(m_data, activation, compiled));
          const auto generation =
              state.active_generation(m_data, activation, resource);
          if (!generation) {
            throw std::logic_error(
                "preconstruction has no active resource generation");
          }
          state.mark_constructed(*generation);
          state.release_active(m_data, activation, resource);
          acquired = false;
        } catch (...) {
          if (acquired) {
            try {
              state.release_active(m_data, activation, resource);
            } catch (...) {
            }
          }
          throw;
        }
      }
      state.mark_outputs_changed(m_data, activation, compiled);
      state.complete_node(m_data, node);
      return;
    }
  }
  if (compiled.kind == detail::OperationKind::construct) {
    if (compiled.accesses.count == 0) {
      throw std::logic_error("construction has no compiled output");
    }
    if (prepare || compiled.memoized) {
      state.acquire_for_execution(
          m_data, activation,
          m_data.accesses[compiled.accesses.offset].resource);
    }
  }
  if (compiled.callback != detail::null_callback_id) {
    if (compiled.kind == detail::OperationKind::callback ||
        compiled.kind == detail::OperationKind::construct) {
      state.prepare_arguments(m_data, activation, compiled);
    }
    auto arguments = compiled.kind == detail::OperationKind::callback ||
                             compiled.kind == detail::OperationKind::construct
                         ? state.arguments(m_data, activation, compiled)
                         : span<void *const>{};
    Lifetime lifetime{};
    if (compiled.kind == detail::OperationKind::construct) {
      const auto resource = m_data.accesses[compiled.accesses.offset].resource;
      if (resource >= m_data.resource_lifetimes.size()) {
        throw std::logic_error(
            "construction resource lifetime is out of range");
      }
      lifetime = m_data.resource_lifetimes[resource].interval;
    }
    auto &callback = m_data.callbacks[compiled.callback];
    if (!callback.runtime) {
      throw std::logic_error("Flow operation is not resolved");
    }
    callback.runtime(lifetime, arguments);
  }
  if (compiled.kind == detail::OperationKind::construct) {
    const auto resource = m_data.accesses[compiled.accesses.offset].resource;
    const auto generation =
        state.active_generation(m_data, activation, resource);
    if (!generation) {
      throw std::logic_error("construction has no active resource generation");
    }
    state.mark_constructed(*generation);
  }
  state.mark_outputs_changed(m_data, activation, compiled);
  state.complete_node(m_data, node);
}

void Program::release_after_node(std::uint32_t activation,
                                 detail::NodeId node) {
  for (ResourceId resource = 0; resource < m_data.resource_lifetimes.size();
       ++resource) {
    const auto &lifetime = m_data.resource_lifetimes[resource];
    const bool release_preconstructed =
        lifetime.preconstructed && !lifetime.retained_by_feedback;
    if ((lifetime.destroy_at_shutdown && !release_preconstructed) ||
        lifetime.release_after.count == 0 ||
        lifetime.release_after.offset + lifetime.release_after.count >
            m_data.release_after.size()) {
      continue;
    }
    detail::NodeId last_frontier = 0;
    for (std::uint32_t index = 0; index < lifetime.release_after.count;
         ++index) {
      last_frontier =
          std::max(last_frontier,
                   m_data.release_after[lifetime.release_after.offset + index]);
    }
    if (last_frontier == node) {
      runtime().release_active(m_data, activation, resource);
    }
  }
}

bool Program::execute_control_iteration(detail::ControlId control,
                                        std::uint32_t activation,
                                        std::uint32_t invocation) {
  if (control >= m_data.controls.size()) {
    throw std::logic_error("structured control is out of range");
  }
  const auto &compiled = m_data.controls[control];
  if (compiled.primary_steps.offset + compiled.primary_steps.count >
          m_data.control_steps.size() ||
      compiled.secondary_steps.offset + compiled.secondary_steps.count >
          m_data.control_steps.size() ||
      compiled.nodes.offset + compiled.nodes.count >
          m_data.control_nodes.size() ||
      compiled.resources.offset + compiled.resources.count >
          m_data.control_resources.size() ||
      compiled.primary_written_resources.offset +
              compiled.primary_written_resources.count >
          m_data.control_written_resources.size() ||
      compiled.secondary_written_resources.offset +
              compiled.secondary_written_resources.count >
          m_data.control_written_resources.size()) {
    throw std::logic_error("structured control ranges are invalid");
  }

  if (compiled.kind == detail::ControlKind::loop && invocation == 0 &&
      !runtime().condition(m_data, activation, compiled.initial_condition)) {
    return false;
  }
  if (compiled.kind == detail::ControlKind::branch && invocation != 0) {
    throw std::logic_error("if control cannot have a subinvocation");
  }

  runtime().reset_control_outputs(
      m_data, activation,
      {m_data.control_nodes.data() + compiled.nodes.offset,
       compiled.nodes.count});

  const bool branch_condition =
      compiled.kind == detail::ControlKind::branch
          ? runtime().condition(m_data, activation, compiled.initial_condition)
          : true;
  const auto steps =
      compiled.kind == detail::ControlKind::branch && !branch_condition
          ? compiled.secondary_steps
          : compiled.primary_steps;
  for (std::uint32_t index = 0; index < steps.count; ++index) {
    const auto &step = m_data.control_steps[steps.offset + index];
    if (step.kind == detail::ControlStepKind::control) {
      execute_control(step.index, activation);
      continue;
    }
    const auto node = static_cast<detail::NodeId>(step.index);
    if (node >= m_data.nodes.size()) {
      throw std::logic_error("structured control node is out of range");
    }
    if (m_data.nodes[node].kind == detail::OperationKind::phi) {
      bool found = false;
      for (const auto &phi : m_data.phis) {
        if (phi.node == node) {
          runtime().select_phi(m_data, activation, phi, invocation != 0);
          found = true;
          break;
        }
      }
      if (!found) {
        throw std::logic_error("structured control phi has no descriptor");
      }
    }
    execute_node(activation, node, true, true);
    release_after_node(activation, node);
  }

  if (compiled.kind == detail::ControlKind::branch) {
    runtime().merge_resource_changes(
        m_data, activation,
        {m_data.control_written_resources.data() +
             compiled.primary_written_resources.offset,
         compiled.primary_written_resources.count},
        {m_data.control_written_resources.data() +
             compiled.secondary_written_resources.offset,
         compiled.secondary_written_resources.count});
    return false;
  }

  const bool repeat =
      runtime().condition(m_data, activation, compiled.repeat_condition);
  runtime().release_previous(
      m_data, activation,
      {m_data.control_resources.data() + compiled.resources.offset,
       compiled.resources.count});
  return repeat;
}

void Program::execute_control(detail::ControlId control,
                              std::uint32_t activation) {
  std::uint32_t invocation = 0;
  while (execute_control_iteration(control, activation, invocation)) {
    ++invocation;
  }
}

detail::ProgramRuntime &Program::runtime() const {
  if (!m_runtime) {
    throw std::logic_error("program has no runtime state");
  }
  return *m_runtime;
}

} // namespace strobe::flow
