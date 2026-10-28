#include <strobe/flow/program.hpp>

#include "strobe/flow/detail/physical_storage_state.hpp"
#include "strobe/flow/detail/program_runtime.hpp"

namespace strobe::flow {

Program::Program() = default;

Program::Program(detail::ProgramArtifact artifact)
    : m_data(std::move(artifact.data)), m_job_plan(std::move(artifact.jobs)),
      m_feedback_plan(std::move(artifact.feedback)) {
  if (!m_data.nodes.empty() && m_data.activation_layout.size == 0) {
    throw std::logic_error("program artifact has no activation layout");
  }
  if (m_feedback_plan.regions.size() != m_data.feedback_regions.size()) {
    throw std::logic_error("program artifact has no feedback submission plan");
  }
  m_runtime = make_box<detail::ProgramRuntime>(allocator_ref{}, m_data);
  m_feedback_iterations.resize(m_data.feedback_regions.size(), 0);
  m_activation_iterations.resize(
      detail::ProgramRuntime::activation_pool_capacity, 0);
  m_serialization.emplace(m_data.resources.size(), allocator_ref{});
  m_initialization_jobs.emplace(m_data.nodes.size(), allocator_ref{});
  m_initialization_job_valid.emplace(m_data.nodes.size(), allocator_ref{});
  if (!m_data.resources.empty()) {
    m_physical_storage = make_box<detail::PhysicalStorageState>(
        allocator_ref{}, m_data, allocator_ref{});
    const auto resource_count = m_data.resources.size();
    m_activation_storage_slots.emplace(
        detail::ProgramRuntime::activation_pool_capacity * resource_count,
        allocator_ref{});
    std::fill(m_activation_storage_slots->begin(),
              m_activation_storage_slots->end(), detail::null_node_id);
  }
  m_feedback_submission.reserve(m_data.feedback_regions.size());
  for (std::size_t region = 0; region < m_data.feedback_regions.size();
       ++region) {
    const auto &compiled_region = m_data.feedback_regions[region];
    const auto &submission = m_feedback_plan.regions[region];
    m_feedback_submission.emplace_back(
        m_data.nodes.size(), m_data.resources.size(),
        compiled_region.max_in_flight, submission.dependency_capacity,
        allocator_ref{});
  }
}

Program::Program(Program &&) noexcept = default;

Program &Program::operator=(Program &&) noexcept = default;

Program::~Program() = default;

const detail::ProgramData &Program::data() const noexcept { return m_data; }

detail::ProgramData &Program::data() noexcept { return m_data; }

const detail::JobPlan &Program::job_plan() const noexcept { return m_job_plan; }

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

void Program::bind_external_typed(std::string_view name, void *pointer,
                                  TypeId type) {
  for (const auto &external : m_data.externals) {
    if (external.name.view() != name) {
      continue;
    }
    if (external.resource >= m_data.resources.size()) {
      throw std::logic_error("external resource type is out of range");
    }
    if (m_data.resources[external.resource].type != type) {
      throw std::invalid_argument(
          "external resource binding type does not match its declaration");
    }
    runtime().bind(m_data, name, pointer);
    return;
  }
  throw std::invalid_argument("external resource name is not declared");
}

void *Program::external(ResourceId resource) const {
  return runtime().external(m_data, resource);
}

bool Program::externals_bound() const {
  return runtime().all_externals_bound();
}

bool Program::libraries_bound() const noexcept {
  const auto callbacks_bound = std::all_of(
      m_data.callbacks.begin(), m_data.callbacks.end(),
      [](const detail::CompiledCallback &callback) {
        return !callback.external || static_cast<bool>(callback.runtime);
      });
  const auto projections_bound = std::all_of(
      m_data.projections.begin(), m_data.projections.end(),
      [](const detail::CompiledProjection &projection) {
        return !projection.external || static_cast<bool>(projection.resolve);
      });
  return callbacks_bound && projections_bound;
}

void Program::bind_library(std::string_view library_id, Library library) {
  if (m_scheduler != nullptr) {
    throw std::logic_error(
        "Flow libraries must be bound before the first submission");
  }
  if (library_id.empty()) {
    throw std::invalid_argument("Flow library binding is invalid");
  }
  Vector<std::size_t, allocator_ref> callback_indices;
  Vector<ExternalOpFunction, allocator_ref> functions;
  Vector<std::size_t, allocator_ref> projection_indices;
  Vector<ExternalProjectionFunction, allocator_ref> projectors;
  for (std::size_t index = 0; index < m_data.callbacks.size(); ++index) {
    const auto &callback = m_data.callbacks[index];
    if (!callback.external ||
        callback.external->library_id.view() != library_id) {
      continue;
    }
    if (library.resolve == nullptr) {
      throw std::invalid_argument("Flow library does not resolve external ops");
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
  for (std::size_t index = 0; index < m_data.projections.size(); ++index) {
    const auto &projection = m_data.projections[index];
    if (!projection.external ||
        projection.external->library_id.view() != library_id) {
      continue;
    }
    if (library.resolve_projection == nullptr) {
      throw std::invalid_argument(
          "Flow library does not resolve external projectors");
    }
    const auto projector = library.resolve_projection(
        library.state, projection.external->symbol.view(),
        projection.external->abi_version);
    if (projector == nullptr) {
      throw std::invalid_argument("Flow external projector was not resolved");
    }
    projection_indices.push_back(index);
    projectors.push_back(projector);
  }
  if (callback_indices.empty() && projection_indices.empty()) {
    throw std::invalid_argument("Flow library id is not declared");
  }
  for (std::size_t resolved = 0; resolved < callback_indices.size();
       ++resolved) {
    auto &callback = m_data.callbacks[callback_indices[resolved]];
    const auto function = functions[resolved];
    const auto abi_version = callback.external->abi_version;
    auto argument_types = callback.argument_types;
    auto argument_accesses = callback.argument_accesses;
    Vector<TypeId, allocator_ref> immediate_types;
    immediate_types.reserve(callback.immediates.count);
    for (std::uint32_t offset = 0; offset < callback.immediates.count;
         ++offset) {
      immediate_types.push_back(
          m_data.immediates[callback.immediates.offset + offset].type);
    }
    callback.runtime = [function, abi_version,
                        argument_types = std::move(argument_types),
                        argument_accesses = std::move(argument_accesses),
                        immediate_types = std::move(immediate_types)](
                           Lifetime lifetime, span<void *const> immediates,
                           span<const std::uint32_t> immediate_counts,
                           span<const ArgumentGroup> argument_groups,
                           span<const std::uint32_t> argument_elements,
                           span<void *const> arguments) noexcept {
      const ExternalOpContext context{
          .abi_version = abi_version,
          .lifetime = lifetime,
          .argument_count = static_cast<std::uint32_t>(arguments.size()),
          .argument_group_count =
              static_cast<std::uint32_t>(argument_groups.size()),
          .argument_element_count =
              static_cast<std::uint32_t>(argument_elements.size()),
          .immediate_count = static_cast<std::uint32_t>(immediates.size()),
          .arguments = arguments.data(),
          .immediates = immediates.data(),
          .immediate_types = immediate_types.data(),
          .immediate_counts = immediate_counts.data(),
          .argument_types = argument_types.data(),
          .argument_accesses = argument_accesses.data(),
          .argument_groups = argument_groups.data(),
          .argument_elements = argument_elements.data(),
      };
      function(context);
    };
  }
  for (std::size_t resolved = 0; resolved < projection_indices.size();
       ++resolved) {
    auto &projection = m_data.projections[projection_indices[resolved]];
    const auto projector = projectors[resolved];
    projection.resolve = [projector](void *owner) noexcept {
      return projector(owner);
    };
  }
}

void Program::release(std::string_view name, std::uint64_t count) {
  ZoneScopedN("Program::release");
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

void Program::prune_completions() {
  if (m_scheduler == nullptr) {
    return;
  }
  std::uint32_t output = 0;
  for (std::uint32_t index = 0; index < m_pending_completion_count; ++index) {
    const auto completion = m_pending_completions[index];
    if (!m_scheduler->completed(completion)) {
      m_pending_completions[output++] = completion;
    }
  }
  m_pending_completion_count = output;
}

void Program::track_completion(sync::job_id completion) {
  prune_completions();
  if (m_pending_completion_count >= m_pending_completions.size()) {
    throw std::logic_error("Flow completion frontier is exhausted");
  }
  m_pending_completions[m_pending_completion_count++] = completion;
}

void Program::wait() {
  if (m_scheduler != nullptr) {
    for (std::uint32_t index = 0; index < m_pending_completion_count; ++index) {
      m_scheduler->wait(m_pending_completions[index]);
    }
    m_pending_completion_count = 0;
  }
  rethrow_execution_error();
}

void Program::submit(sync::JobScheduler *scheduler) {
  ZoneScopedN("Program::submit");
  struct PlannedJob {
    Program *program;
    std::uint32_t activation;
    detail::JobId job;

    void operator()() noexcept {
      ZoneScopedN("flow-planned-job");
      auto &runtime = program->runtime();
      if (runtime.failed()) {
        return;
      }
      try {
        program->execute_job(activation, job);
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
      ZoneScopedN("flow-control-job");
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
      ZoneScopedN("flow-release-resource");
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
      ZoneScopedN("flow-finish-activation");
      auto &runtime = program->runtime();
      try {
        runtime.finish_execution(program->m_data, activation,
                                 activation_complete);
      } catch (...) {
        runtime.record_error(std::current_exception());
      }
    }
  };

  static_assert(sync::job_fn<PlannedJob>);
  static_assert(sync::job_fn<ControlJob>);
  static_assert(sync::job_fn<ReleaseJob>);
  static_assert(sync::job_fn<FinishJob>);

  if (scheduler == nullptr) {
    throw std::invalid_argument("Flow scheduler cannot be null");
  }
  if (!libraries_bound()) {
    throw std::logic_error("not all Flow external op libraries are bound");
  }
  if (!m_data.feedback_regions.empty() && m_data.yields.empty() &&
      m_data.controls.empty()) {
    submit_feedback_regions(scheduler);
    return;
  }
  if (m_scheduler == nullptr) {
    m_scheduler = scheduler;
  } else if (m_scheduler != scheduler) {
    throw std::invalid_argument(
        "Flow program must use the scheduler from its first submission");
  }
  runtime().clear_error();
  prune_completions();
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
        m_data.node_storage.predecessors.size()) {
      throw std::logic_error("compiled node predecessors are out of range");
    }
    for (const auto predecessor : m_data.predecessors_of(compiled)) {
      if (predecessor >= node) {
        throw std::logic_error(
            "compiled jobs must be stored in dependency order");
      }
    }
    if (compiled.accesses.offset + compiled.accesses.count >
        m_data.node_storage.accesses.size()) {
      throw std::logic_error("compiled node accesses are out of range");
    }
  }

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
    releases.reserve(m_data.resources.size());
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
                m_data.control_nodes.size() ||
            compiled_control.nodes.count == 0) {
          throw std::logic_error("top-level structured control is invalid");
        }
        const auto first_control_node =
            m_data.control_nodes[compiled_control.nodes.offset];
        bool control_initialization = true;
        for (std::uint32_t member_index = 0;
             member_index < compiled_control.nodes.count; ++member_index) {
          const auto member =
              m_data
                  .control_nodes[compiled_control.nodes.offset + member_index];
          control_initialization &= m_data.nodes[member].initialization;
        }
        if (control_initialization &&
            (*m_initialization_job_valid)[first_control_node]) {
          const auto job = (*m_initialization_jobs)[first_control_node];
          control_submitted[control] = true;
          for (std::uint32_t member_index = 0;
               member_index < compiled_control.nodes.count; ++member_index) {
            const auto member =
                m_data.control_nodes[compiled_control.nodes.offset +
                                     member_index];
            jobs[member] = job;
            job_valid[member] = true;
          }
          continue;
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
          for (const auto predecessor : m_data.predecessors_of(member_node)) {
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
                  m_data.node_storage.temporal_predecessors
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
          if (control_initialization) {
            (*m_initialization_jobs)[member] = job;
            m_initialization_job_valid->set(member);
          }
        }
        continue;
      }
      if (job_valid[node] || node >= m_job_plan.job_of_node.size()) {
        if (node >= m_job_plan.job_of_node.size()) {
          throw std::logic_error("node has no planned job");
        }
        continue;
      }
      const auto planned_id = m_job_plan.job_of_node[node];
      if (planned_id >= m_job_plan.jobs.size()) {
        throw std::logic_error("planned job is out of range");
      }
      const auto &planned = m_job_plan.jobs[planned_id];
      const auto planned_nodes = m_job_plan.nodes_of(planned);
      if (planned_nodes.empty() || planned_nodes.front() != node ||
          planned_nodes.back() >= end_node) {
        throw std::logic_error("planned job crosses a submission boundary");
      }
      const bool job_initialization = m_data.nodes[planned_nodes.front()].initialization;
      for (const auto member : planned_nodes) {
        if (m_data.nodes[member].initialization != job_initialization) {
          throw std::logic_error(
              "planned job crosses an execution region boundary");
        }
      }
      if (job_initialization &&
          (*m_initialization_job_valid)[planned_nodes.front()]) {
        const auto job = (*m_initialization_jobs)[planned_nodes.front()];
        for (const auto member : planned_nodes) {
          jobs[member] = job;
          job_valid[member] = true;
        }
        continue;
      }
      for (const auto member : planned_nodes) {
        const auto &compiled = m_data.nodes[member];
        if (compiled.kind == detail::OperationKind::construct &&
            !compiled.memoized && compiled.accesses.count != 0 &&
            !m_data.resources[m_data.accesses_of(compiled).front().resource]
                 .lifetime.preconstructed) {
          runtime().acquire_for_execution(
              m_data, activation,
              m_data.accesses_of(compiled).front().resource);
        } else if (compiled.kind == detail::OperationKind::construct &&
                   compiled.accesses.count == 0) {
          throw std::logic_error("construction has no compiled output");
        } else if (compiled.kind == detail::OperationKind::discard &&
                   compiled.accesses.count != 0 &&
                   m_data
                       .resources[m_data.accesses_of(compiled).front().resource]
                       .lifetime.preconstructed) {
          runtime().acquire_for_execution(
              m_data, activation,
              m_data.accesses_of(compiled).front().resource);
        } else if (compiled.kind == detail::OperationKind::phi) {
          bool found = false;
          for (const auto &phi : m_data.phis) {
            if (phi.node == member) {
              runtime().select_phi(m_data, activation, phi);
              found = true;
              break;
            }
          }
          if (!found) {
            throw std::logic_error("phi node has no runtime descriptor");
          }
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
      if (m_data.yields.empty()) {
        for (const auto predecessor_job : m_job_plan.predecessors_of(planned)) {
          if (predecessor_job >= m_job_plan.jobs.size()) {
            throw std::logic_error("planned job predecessor is out of range");
          }
          const auto predecessor_nodes = m_job_plan.nodes_of(predecessor_job);
          if (predecessor_nodes.empty()) {
            throw std::logic_error("planned job predecessor is malformed");
          }
          const auto predecessor = predecessor_nodes.front();
          if (predecessor >= job_valid.size() || !job_valid[predecessor]) {
            throw std::logic_error("planned job predecessor was not submitted");
          }
          add_dependency(jobs[predecessor]);
        }
      }
      for (const auto member : planned_nodes) {
        const auto &compiled = m_data.nodes[member];
        if (!m_data.yields.empty()) {
          for (const auto predecessor : m_data.predecessors_of(compiled)) {
            if (m_job_plan.job_of_node[predecessor] == planned_id) {
              continue;
            }
            if (predecessor >= first_node) {
              if (!job_valid[predecessor]) {
                throw std::logic_error("node predecessor was not submitted");
              }
              add_dependency(jobs[predecessor]);
            }
          }
        }
        if (!has_previous_activation) {
          continue;
        }
        for (const auto predecessor :
             m_data.temporal_predecessors_of(compiled)) {
          if (previous_activation_job_valid[predecessor]) {
            add_dependency(previous_activation_jobs[predecessor]);
          }
        }
        if (compiled.memoized && previous_activation_job_valid[member]) {
          add_dependency(previous_activation_jobs[member]);
        }
        for (const auto &control_descriptor : m_data.controls) {
          if (control_descriptor.parent != detail::null_control_id ||
              control_descriptor.initial_condition_node != member) {
            continue;
          }
          if (control_descriptor.nodes.offset + control_descriptor.nodes.count >
                  m_data.control_nodes.size() ||
              control_descriptor.nodes.count == 0) {
            throw std::logic_error("structured control nodes are invalid");
          }
          const auto control_member =
              m_data.control_nodes[control_descriptor.nodes.offset];
          if (previous_activation_job_valid[control_member]) {
            add_dependency(previous_activation_jobs[control_member]);
          }
        }
      }
      scheduler->submit(job, PlannedJob{this, activation, planned_id});
      for (const auto member : planned_nodes) {
        jobs[member] = job;
        job_valid[member] = true;
        if (job_initialization) {
          (*m_initialization_jobs)[member] = job;
          m_initialization_job_valid->set(member);
        }
      }

      for (const auto resource : m_job_plan.released_resources_of(planned)) {
        const auto &lifetime = m_data.resources[resource].lifetime;
        const auto release = scheduler->acquire();
        Vector<sync::job_id, allocator_ref> release_dependencies;
        for (std::uint32_t index = 0; index < lifetime.release_after.count;
             ++index) {
          const auto frontier =
              m_data.release_after[lifetime.release_after.offset + index];
          if (frontier >= end_node) {
            throw std::logic_error("release frontier is out of range");
          }
          if (frontier >= first_node) {
            if (std::find(release_dependencies.begin(),
                          release_dependencies.end(),
                          jobs[frontier]) == release_dependencies.end()) {
              release_dependencies.push_back(jobs[frontier]);
              scheduler->addDependency(release, jobs[frontier]);
            }
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
      for (const auto successor : m_data.successors_of(compiled)) {
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
    track_completion(completion);

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
}

void Program::submit_feedback_regions(sync::JobScheduler *scheduler) {
  struct InitializationJob {
    Program *program;
    std::uint32_t activation;

    void operator()() noexcept {
      ZoneScopedN("flow-initialization");
      try {
        program->execute_initialization(activation);
      } catch (...) {
        program->runtime().record_error(std::current_exception());
      }
    }
  };
  struct RegionPlannedJob {
    Program *program;
    std::uint32_t activation;
    detail::JobId job;

    void operator()() noexcept {
      ZoneScopedN("flow-feedback-job");
      try {
        program->execute_feedback_job(activation, job);
      } catch (...) {
        program->runtime().record_error(std::current_exception());
      }
    }
  };
  struct ReleaseJob {
    Program *program;
    std::uint32_t activation;
    ResourceId resource;

    void operator()() noexcept {
      ZoneScopedN("flow-release-resource");
      try {
        program->runtime().release_active(program->m_data, activation,
                                          resource);
      } catch (...) {
        program->runtime().record_error(std::current_exception());
      }
    }
  };
  struct FinishJob {
    Program *program;
    std::uint32_t activation;

    void operator()() noexcept {
      ZoneScopedN("flow-finish-feedback-activation");
      try {
        program->runtime().finish_execution(program->m_data, activation, true);
      } catch (...) {
        program->runtime().record_error(std::current_exception());
      }
    }
  };
  static_assert(sync::job_fn<InitializationJob>);
  static_assert(sync::job_fn<RegionPlannedJob>);
  static_assert(sync::job_fn<ReleaseJob>);
  static_assert(sync::job_fn<FinishJob>);

  if (m_scheduler == nullptr) {
    m_scheduler = scheduler;
  } else if (m_scheduler != scheduler) {
    throw std::invalid_argument(
        "Flow program must use the scheduler from its first submission");
  }
  runtime().clear_error();
  prune_completions();

  if (!m_initialization_submitted) {
    const auto activation = runtime().begin_execution(m_data);
    const auto initialization = scheduler->acquire();
    scheduler->submit(initialization, InitializationJob{this, activation});
    track_completion(initialization);
    m_initialization_completion = initialization;
    m_initialization_submitted = true;
  }

  for (std::uint32_t region = 0; region < m_data.feedback_regions.size();
       ++region) {
    const auto &compiled_region = m_data.feedback_regions[region];
    const auto &submission_plan = m_feedback_plan.regions[region];
    auto &state = m_feedback_submission[region];
    while (runtime().try_consume_tokens(m_data, compiled_region)) {
      const auto activation = runtime().begin_execution(m_data);
      const auto resource_count = m_data.resources.size();
      std::fill_n(m_activation_storage_slots->data() +
                      activation * resource_count,
                  resource_count, detail::null_node_id);
      const auto iteration =
          static_cast<std::uint32_t>(m_feedback_iterations[region]++);
      m_activation_iterations[activation] = iteration;
      const auto previous_jobs = state.previous_jobs;
      const auto current_jobs = previous_jobs ^ 1U;
      auto &jobs = state.jobs[current_jobs];
      auto &job_valid = state.job_valid[current_jobs];
      const auto &previous_job_ids = state.jobs[previous_jobs];
      const auto &previous_job_valid = state.job_valid[previous_jobs];
      job_valid.reset_all();
      state.retained_acquired.reset_all();
      state.physical_resources_acquired.reset_all();
      state.physical_frontiers_started.reset_all();
      std::size_t release_count = 0;

      const auto physical_slot_for = [&](ResourceId resource) {
        const auto logical_slot =
            (*m_activation_storage_slots)[activation * resource_count +
                                          resource];
        if (logical_slot == detail::null_node_id) {
          throw std::logic_error("physical storage resource was not acquired");
        }
        return m_physical_storage->physical_slot(resource, logical_slot);
      };

      const auto replace_physical_frontier = [&](ResourceId resource,
                                                 sync::job_id frontier) {
        m_physical_storage->replace_frontier(physical_slot_for(resource),
                                             frontier);
      };

      const auto append_physical_frontier = [&](ResourceId resource,
                                                sync::job_id frontier) {
        m_physical_storage->append_frontier(physical_slot_for(resource),
                                            frontier);
      };

      for (std::uint32_t index = 0; index < submission_plan.jobs.count;
           ++index) {
        const auto planned_id =
            m_feedback_plan.jobs[submission_plan.jobs.offset + index];
        if (planned_id >= m_job_plan.jobs.size()) {
          throw std::logic_error("feedback planned job is out of range");
        }
        const auto &planned = m_job_plan.jobs[planned_id];
        const auto planned_nodes = m_job_plan.nodes_of(planned);
        if (planned_nodes.empty()) {
          throw std::logic_error("feedback planned job is malformed");
        }
        for (const auto planned_node : planned_nodes) {
          const auto &compiled = m_data.nodes[planned_node];
          if (planned_node < m_data.control_of_node.size() &&
              m_data.control_of_node[planned_node] != detail::null_control_id) {
            throw std::logic_error("structured control inside an external "
                                   "feedback region is not yet supported");
          }
          if (compiled.kind == detail::OperationKind::yield) {
            throw std::logic_error(
                "yield inside an external feedback region is unsupported");
          }
        }
        const auto job = scheduler->acquire();
        std::size_t dependency_count = 0;
        const auto add_dependency = [&](sync::job_id target,
                                        sync::job_id dependency) {
          for (std::size_t dependency_index = 0;
               dependency_index < dependency_count; ++dependency_index) {
            if (state.dependencies[dependency_index] == dependency) {
              return;
            }
          }
          if (dependency_count >= state.dependencies.size()) {
            throw std::logic_error("feedback dependency scratch is exhausted");
          }
          state.dependencies[dependency_count++] = dependency;
          scheduler->addDependency(target, dependency);
        };
        for (const auto predecessor_job : m_job_plan.predecessors_of(planned)) {
          if (predecessor_job >= m_job_plan.jobs.size()) {
            throw std::logic_error(
                "feedback planned job predecessor is out of range");
          }
          const auto predecessor_nodes = m_job_plan.nodes_of(predecessor_job);
          if (predecessor_nodes.empty()) {
            throw std::logic_error(
                "feedback planned job predecessor is malformed");
          }
          const auto predecessor = predecessor_nodes.front();
          if (predecessor >= m_feedback_plan.region_of_node.size()) {
            throw std::logic_error(
                "feedback planned job predecessor is out of range");
          }
          if (m_feedback_plan.region_of_node[predecessor] == region) {
            if (!job_valid[predecessor]) {
              throw std::logic_error(
                  "feedback region predecessor was not submitted");
            }
            add_dependency(job, jobs[predecessor]);
          } else if (m_initialization_completion) {
            add_dependency(job, *m_initialization_completion);
          }
        }
        for (const auto planned_node : planned_nodes) {
          const auto &planned_compiled = m_data.nodes[planned_node];
          for (const auto predecessor :
               m_data.temporal_predecessors_of(planned_compiled)) {
            if (predecessor < previous_job_valid.size() &&
                previous_job_valid[predecessor]) {
              add_dependency(job, previous_job_ids[predecessor]);
            }
          }
          if (planned_compiled.memoized &&
              previous_job_valid[planned_node]) {
            add_dependency(job, previous_job_ids[planned_node]);
          }
        }
        if (iteration == 0 && m_initialization_completion) {
          add_dependency(job, *m_initialization_completion);
        }
        auto &serialization = *m_serialization;
        for (const auto planned_node : planned_nodes) {
          const auto &planned_compiled = m_data.nodes[planned_node];
          for (const auto &access : m_data.accesses_of(planned_compiled)) {
            if (access.purpose != AccessPurpose::serialization) {
              continue;
            }
            if (access.resource >= serialization.valid.size()) {
              throw std::logic_error(
                  "feedback serialization resource is out of range");
            }
            if (serialization.valid[access.resource]) {
              add_dependency(job, serialization.jobs[access.resource]);
            }
          }
        }
        const auto register_retained_acquire = [&](ResourceId resource) {
          if (resource >= m_data.resources.size() ||
              !m_data.resources[resource].lifetime.retained_by_feedback) {
            return;
          }
          if (state.retained_acquired[resource]) {
            return;
          }
          if (resource >= state.retained_acquire_valid.size() ||
              state.max_in_flight == 0) {
            throw std::logic_error(
                "retained feedback resource state is out of range");
          }
          if (state.retained_acquire_valid[resource]) {
            add_dependency(job, state.retained_acquire_jobs[resource]);
          }
          if (state.retained_writer_valid[resource]) {
            add_dependency(job, state.retained_writer_jobs[resource]);
          }
          const auto slot_count = m_data.resources[resource].slots.slot_count;
          const auto acquisition_count =
              state.retained_acquisition_counts[resource];
          if (slot_count != 0 && acquisition_count >= slot_count) {
            const auto release_count = acquisition_count - (slot_count - 1);
            const auto history_index = resource * state.max_in_flight +
                                       release_count % state.max_in_flight;
            if (history_index >= state.retained_completion_valid.size() ||
                !state.retained_completion_valid[history_index]) {
              throw std::logic_error(
                  "retained feedback slot has no completion frontier");
            }
            add_dependency(job,
                           state.retained_completion_history[history_index]);
          }
          state.retained_acquire_jobs[resource] = job;
          state.retained_acquire_valid.set(resource);
          state.retained_acquired.set(resource);
        };
        const auto register_physical_storage = [&](ResourceId resource) {
          if (resource >= state.physical_resources_acquired.size()) {
            throw std::logic_error("physical storage resource is out of range");
          }
          if (state.physical_resources_acquired[resource]) {
            return;
          }
          const auto logical_slot = m_physical_storage->acquire(resource);
          if (logical_slot == detail::null_node_id) {
            return;
          }
          const auto physical_slot =
              m_physical_storage->physical_slot(resource, logical_slot);
          for (const auto frontier :
               m_physical_storage->frontier(physical_slot)) {
            add_dependency(job, frontier);
          }
          state.physical_resources_acquired.set(resource);
          (*m_activation_storage_slots)[activation * resource_count +
                                        resource] = logical_slot;
          replace_physical_frontier(resource, job);
        };
        for (const auto planned_node : planned_nodes) {
          const auto &planned_compiled = m_data.nodes[planned_node];
          const bool acquires_storage =
              planned_compiled.kind == detail::OperationKind::construct ||
              planned_compiled.kind == detail::OperationKind::discard;
          if (!acquires_storage || planned_compiled.accesses.count == 0) {
            continue;
          }
          const auto resource =
              m_data.accesses_of(planned_compiled).front().resource;
          register_retained_acquire(resource);
          register_physical_storage(resource);
        }
        scheduler->submit(job, RegionPlannedJob{this, activation, planned_id});
        for (const auto planned_node : planned_nodes) {
          jobs[planned_node] = job;
          job_valid.set(planned_node);
          const auto &planned_compiled = m_data.nodes[planned_node];
          for (const auto &access : m_data.accesses_of(planned_compiled)) {
            if (access.purpose == AccessPurpose::serialization) {
              serialization.jobs[access.resource] = job;
              serialization.valid.set(access.resource);
            }
            if (access.output != detail::null_value_id &&
                access.resource < m_data.resources.size() &&
                m_data.resources[access.resource]
                    .lifetime.retained_by_feedback) {
              state.retained_writer_jobs[access.resource] = job;
              state.retained_writer_valid.set(access.resource);
            }
          }
        }

        for (const auto resource : m_job_plan.resource_frontiers_of(planned)) {
          if (!state.physical_resources_acquired[resource]) {
            continue;
          }
          if (!state.physical_frontiers_started[resource]) {
            const auto physical_slot = physical_slot_for(resource);
            m_physical_storage->clear_frontier(physical_slot);
            state.physical_frontiers_started.set(resource);
          }
          append_physical_frontier(resource, job);
        }

        for (const auto resource : m_job_plan.released_resources_of(planned)) {
          const auto &lifetime = m_data.resources[resource].lifetime;
          const auto release = scheduler->acquire();
          dependency_count = 0;
          for (std::uint32_t frontier_index = 0;
               frontier_index < lifetime.release_after.count;
               ++frontier_index) {
            const auto frontier =
                m_data.release_after[lifetime.release_after.offset +
                                     frontier_index];
            if (frontier >= m_feedback_plan.region_of_node.size() ||
                m_feedback_plan.region_of_node[frontier] != region ||
                !job_valid[frontier]) {
              throw std::logic_error(
                  "resource release crosses a feedback region frontier");
            }
            add_dependency(release, jobs[frontier]);
          }
          scheduler->submit(release, ReleaseJob{this, activation, resource});
          if (state.physical_resources_acquired[resource]) {
            replace_physical_frontier(resource, release);
          }
          if (release_count >= state.releases.size()) {
            throw std::logic_error("feedback release scratch is exhausted");
          }
          state.releases[release_count++] = release;
        }
      }

      const auto completion = scheduler->acquire();
      std::size_t dependency_count = 0;
      const auto add_completion_dependency = [&](sync::job_id dependency) {
        for (std::size_t dependency_index = 0;
             dependency_index < dependency_count; ++dependency_index) {
          if (state.dependencies[dependency_index] == dependency) {
            return;
          }
        }
        if (dependency_count >= state.dependencies.size()) {
          throw std::logic_error("feedback dependency scratch is exhausted");
        }
        state.dependencies[dependency_count++] = dependency;
        scheduler->addDependency(completion, dependency);
      };
      for (std::uint32_t index = 0; index < submission_plan.terminal_jobs.count;
           ++index) {
        const auto terminal_job =
            m_feedback_plan
                .terminal_jobs[submission_plan.terminal_jobs.offset + index];
        if (terminal_job >= m_job_plan.jobs.size() ||
            m_job_plan.nodes_of(terminal_job).empty()) {
          throw std::logic_error("feedback terminal job is invalid");
        }
        const auto node = m_job_plan.nodes_of(terminal_job).front();
        add_completion_dependency(jobs[node]);
      }
      for (std::size_t release_index = 0; release_index < release_count;
           ++release_index) {
        add_completion_dependency(state.releases[release_index]);
      }
      scheduler->submit(completion, FinishJob{this, activation});
      track_completion(completion);
      state.previous_jobs = current_jobs;
      for (ResourceId resource = 0;
           resource < state.physical_resources_acquired.size(); ++resource) {
        if (state.physical_resources_acquired[resource]) {
          m_physical_storage->advance(resource);
        }
      }
      for (ResourceId resource = 0; resource < state.retained_acquired.size();
           ++resource) {
        if (!state.retained_acquired[resource]) {
          continue;
        }
        const auto acquisition_count =
            state.retained_acquisition_counts[resource];
        const auto history_index = resource * state.max_in_flight +
                                   acquisition_count % state.max_in_flight;
        if (history_index >= state.retained_completion_history.size()) {
          throw std::logic_error(
              "retained feedback completion history is out of range");
        }
        state.retained_completion_history[history_index] = completion;
        state.retained_completion_valid.set(history_index);
        ++state.retained_acquisition_counts[resource];
      }
    }
  }
}

void Program::execute_initialization(std::uint32_t activation) {
  for (detail::NodeId node = 0; node < m_data.nodes.size(); ++node) {
    bool in_feedback_region = false;
    for (const auto &region : m_data.feedback_regions) {
      const auto begin =
          m_data.feedback_region_nodes.begin() + region.nodes.offset;
      const auto end = begin + region.nodes.count;
      in_feedback_region |= std::find(begin, end, node) != end;
    }
    if (in_feedback_region) {
      continue;
    }
    if (node < m_data.control_of_node.size() &&
        m_data.control_of_node[node] != detail::null_control_id) {
      throw std::logic_error("structured control with independent feedback "
                             "regions is not yet supported");
    }
    execute_node(activation, node, true);
    release_after_node(activation, node);
  }
  runtime().finish_execution(m_data, activation, true);
}

void Program::execute_feedback_node(std::uint32_t activation,
                                    detail::NodeId node) {
  if (activation >= m_activation_iterations.size()) {
    throw std::logic_error("feedback activation is out of range");
  }
  const auto &compiled = m_data.nodes[node];
  if (compiled.kind == detail::OperationKind::phi) {
    for (const auto &phi : m_data.phis) {
      if (phi.node == node) {
        runtime().select_phi(m_data, activation, phi,
                             m_activation_iterations[activation] != 0);
        return;
      }
    }
    throw std::logic_error("feedback phi has no region descriptor");
  }
  if (compiled.kind != detail::OperationKind::acquire) {
    bool prepared = false;
    if (compiled.kind == detail::OperationKind::discard &&
        compiled.accesses.count != 0) {
      const auto resource = m_data.accesses_of(compiled).front().resource;
      if (resource < m_data.resources.size() &&
          m_data.resources[resource].lifetime.preconstructed) {
        const auto preferred_slot =
            (*m_activation_storage_slots)[activation * m_data.resources.size() +
                                          resource];
        runtime().acquire_for_execution(m_data, activation, resource,
                                        preferred_slot);
        prepared = true;
      }
    }
    execute_node(activation, node, !prepared);
  }
}

void Program::execute_job(std::uint32_t activation, detail::JobId job) {
  if (job >= m_job_plan.jobs.size()) {
    throw std::logic_error("planned job is out of range");
  }
  for (const auto node : m_job_plan.nodes_of(job)) {
    execute_node(activation, node, false);
  }
}

void Program::execute_feedback_job(std::uint32_t activation,
                                   detail::JobId job) {
  if (job >= m_job_plan.jobs.size()) {
    throw std::logic_error("feedback planned job is out of range");
  }
  for (const auto node : m_job_plan.nodes_of(job)) {
    execute_feedback_node(activation, node);
  }
}

void Program::execute_node(std::uint32_t activation, detail::NodeId node,
                           bool prepare, bool force) {
  if (node >= m_data.nodes.size()) {
    throw std::logic_error("compiled node is out of range");
  }
  auto &compiled = m_data.nodes[node];
  auto &state = runtime();
  if (!force && compiled.memoized &&
      !state.should_execute(m_data, activation, node)) {
    if (compiled.memoized) {
      state.retain_outputs(m_data, activation, compiled);
    }
    return;
  }
  if (compiled.kind == detail::OperationKind::cutoff) {
    for (std::size_t index = 0; index < m_data.cutoffs.size(); ++index) {
      if (m_data.cutoffs[index].node == node) {
        state.evaluate_cutoff(m_data, activation, index);
        return;
      }
    }
    throw std::logic_error("cutoff node has no runtime descriptor");
  }
  if (compiled.kind == detail::OperationKind::construct &&
      compiled.accesses.count != 0) {
    const auto resource = m_data.accesses_of(compiled).front().resource;
    if (resource < m_data.resources.size() &&
        m_data.resources[resource].lifetime.preconstructed) {
      if (compiled.callback == detail::null_callback_id ||
          compiled.callback >= m_data.callbacks.size()) {
        throw std::logic_error(
            "preconstructed resource has no construction callback");
      }
      auto &callback = m_data.callbacks[compiled.callback];
      const auto slots = m_data.resources[resource].slots.slot_count;
      for (std::uint32_t slot = 0; slot < slots; ++slot) {
        state.acquire_for_preconstruction(m_data, activation, resource);
        bool acquired = true;
        try {
          state.prepare_arguments(m_data, activation, compiled);
          callback.runtime(m_data.resources[resource].lifetime.interval,
                           state.immediates(callback),
                           state.immediate_counts(callback),
                           m_data.argument_groups_of(callback),
                           m_data.argument_elements_of(callback),
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
      state.retain_feedback_outputs(m_data, activation, compiled);
      return;
    }
  }
  if (compiled.kind == detail::OperationKind::construct) {
    if (compiled.accesses.count == 0) {
      throw std::logic_error("construction has no compiled output");
    }
    if (compiled.callback == detail::null_callback_id) {
      throw std::logic_error("construction has no runtime callback");
    }
    if (prepare || compiled.memoized) {
      const auto resource = m_data.accesses_of(compiled).front().resource;
      const auto preferred_slot =
          (*m_activation_storage_slots)[activation * m_data.resources.size() +
                                        resource];
      state.acquire_for_execution(m_data, activation, resource, preferred_slot);
    }
  } else if (prepare && compiled.kind == detail::OperationKind::discard &&
             compiled.accesses.count != 0) {
    const auto resource = m_data.accesses_of(compiled).front().resource;
    if (resource < m_data.resources.size() &&
        m_data.resources[resource].lifetime.preconstructed) {
      const auto preferred_slot =
          (*m_activation_storage_slots)[activation * m_data.resources.size() +
                                        resource];
      state.acquire_for_execution(m_data, activation, resource, preferred_slot);
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
      const auto resource = m_data.accesses_of(compiled).front().resource;
      if (resource >= m_data.resources.size()) {
        throw std::logic_error(
            "construction resource lifetime is out of range");
      }
      lifetime = m_data.resources[resource].lifetime.interval;
    }
    auto &callback = m_data.callbacks[compiled.callback];
    if (!callback.runtime) {
      throw std::logic_error("Flow operation is not resolved");
    }
    callback.runtime(lifetime, state.immediates(callback),
                     state.immediate_counts(callback),
                     m_data.argument_groups_of(callback),
                     m_data.argument_elements_of(callback), arguments);
  }
  if (compiled.kind == detail::OperationKind::construct) {
    const auto resource = m_data.accesses_of(compiled).front().resource;
    const auto generation =
        state.active_generation(m_data, activation, resource);
    if (!generation) {
      throw std::logic_error("construction has no active resource generation");
    }
    state.mark_constructed(*generation);
  }
  state.mark_outputs_changed(m_data, activation, compiled);
  state.retain_feedback_outputs(m_data, activation, compiled);
}

void Program::release_after_node(std::uint32_t activation,
                                 detail::NodeId node) {
  if (node >= m_job_plan.job_of_node.size()) {
    throw std::logic_error("release node has no planned job");
  }
  const auto job = m_job_plan.job_of_node[node];
  if (job >= m_job_plan.jobs.size()) {
    throw std::logic_error("release node job is out of range");
  }
  const auto &planned = m_job_plan.jobs[job];
  const auto planned_nodes = m_job_plan.nodes_of(planned);
  if (planned_nodes.empty() || planned_nodes.back() != node) {
    return;
  }
  for (const auto resource : m_job_plan.released_resources_of(planned)) {
    runtime().release_active(m_data, activation, resource);
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
