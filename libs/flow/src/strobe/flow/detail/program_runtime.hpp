#pragma once

#include <strobe/flow/program.hpp>

#include <algorithm>
#include <condition_variable>
#include <exception>
#include <mutex>
#include <new>
#include <stdexcept>
#include <string>

namespace strobe::flow::detail {

struct RuntimeSlot {
  std::uint64_t generation = 0;
  bool occupied = false;
  bool constructed = false;
};

struct RuntimeResource {
  std::size_t offset = null_storage_offset;
  std::size_t stride = 0;
  std::uint32_t first_slot = 0;
  std::uint32_t slot_count = 0;
  std::uint64_t next_generation = 1;
  void (*destroy)(void *) noexcept = nullptr;
  bool preconstructed = false;
};

struct RuntimeCutoff {
  void *storage = nullptr;
  std::size_t allocation_size = 0;
  std::size_t allocation_alignment = 1;
  void (*destroy)(void *) noexcept = nullptr;
  bool initialized = false;
};

class ProgramRuntime {
public:
  explicit ProgramRuntime(const ProgramData &data)
      : m_external_bindings(data.externals.size(), nullptr, allocator_ref{}),
        m_semaphore_tokens(data.semaphore_names.size(), 0, allocator_ref{}) {
    m_activation_capacity = data.activation_layout.capacity;
    m_activation_stride = data.activation_layout.stride;
    m_activation_generations_offset = data.activation_layout.generations_offset;
    m_activation_previous_generations_offset =
        data.activation_layout.previous_generations_offset;
    m_resources.reserve(data.resource_slots.size());
    std::uint32_t first_slot = 0;
    for (ResourceId resource = 0; resource < data.resource_slots.size();
         ++resource) {
      const auto &layout = data.resource_slots[resource];
      auto destroy = static_cast<void (*)(void *) noexcept>(nullptr);
      if (resource < data.resource_lifetimes.size()) {
        destroy = data.resource_lifetimes[resource].destroy;
      }
      m_resources.push_back(RuntimeResource{
          .offset = layout.offset,
          .stride = layout.slot_stride,
          .first_slot = first_slot,
          .slot_count = layout.slot_count,
          .next_generation = 1,
          .destroy = destroy,
          .preconstructed = resource < data.resource_lifetimes.size() &&
                            data.resource_lifetimes[resource].preconstructed,
      });
      if (layout.slot_count >
          std::numeric_limits<std::uint32_t>::max() - first_slot) {
        throw std::length_error("Flow runtime slot table overflow");
      }
      first_slot += layout.slot_count;
    }
    m_slots.resize(first_slot);
    m_active_generations.resize(data.resource_slots.size(), std::nullopt);
    m_completed_once.resize(data.nodes.size(), false);
    m_once_claimed.resize(data.nodes.size(), false);
    m_once_claim_sequences.resize(data.nodes.size(),
                                  std::numeric_limits<std::uint64_t>::max());
    m_activation_occupied.resize(data.activation_layout.capacity, false);
    m_activation_running.resize(data.activation_layout.capacity, false);
    m_activation_sequences.resize(data.activation_layout.capacity, 0);

    try {
      m_cutoffs.reserve(data.cutoffs.size());
      for (const auto &cutoff : data.cutoffs) {
        const auto alignment = std::max(cutoff.alignment, alignof(void *));
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
          throw std::logic_error("cutoff alignment is invalid");
        }
        if (cutoff.size >
            std::numeric_limits<std::size_t>::max() - (alignment - 1)) {
          throw std::length_error("cutoff storage size overflow");
        }
        const auto size = (cutoff.size + alignment - 1) & ~(alignment - 1);
        auto *storage = allocator_ref{}.allocate(size, alignment);
        if (storage == nullptr) {
          throw std::bad_alloc();
        }
        m_cutoffs.push_back(RuntimeCutoff{
            .storage = storage,
            .allocation_size = size,
            .allocation_alignment = alignment,
            .destroy = cutoff.destroy,
            .initialized = false,
        });
      }

      if (data.storage_size != 0) {
        m_storage_alignment = std::max(data.storage_alignment, alignof(void *));
        const auto remainder = data.storage_size % m_storage_alignment;
        m_storage_size = data.storage_size;
        if (remainder != 0) {
          if (m_storage_size > std::numeric_limits<std::size_t>::max() -
                                   (m_storage_alignment - remainder)) {
            throw std::length_error("Flow runtime storage size overflow");
          }
          m_storage_size += m_storage_alignment - remainder;
        }
        m_storage =
            allocator_ref{}.allocate(m_storage_size, m_storage_alignment);
        if (m_storage == nullptr) {
          throw std::bad_alloc();
        }
      }
      if (data.activation_layout.size != 0) {
        m_activation_storage_size = data.activation_layout.size;
        m_activation_storage_alignment =
            std::max(data.activation_layout.alignment, alignof(void *));
        m_activation_storage = allocator_ref{}.allocate(
            m_activation_storage_size, m_activation_storage_alignment);
        if (m_activation_storage == nullptr) {
          throw std::bad_alloc();
        }
        for (std::uint32_t activation = 0;
             activation < data.activation_layout.capacity; ++activation) {
          auto *base = static_cast<std::byte *>(m_activation_storage) +
                       data.activation_layout.stride * activation;
          std::fill_n(reinterpret_cast<void **>(
                          base + data.activation_layout.values_offset),
                      data.values.size(), nullptr);
          std::fill_n(reinterpret_cast<void **>(
                          base + data.activation_layout.arguments_offset),
                      data.accesses.size(), nullptr);
          auto *generations =
              reinterpret_cast<std::optional<ResourceGeneration> *>(
                  base + data.activation_layout.generations_offset);
          for (std::size_t resource = 0; resource < data.resource_slots.size();
               ++resource) {
            std::construct_at(generations + resource, std::nullopt);
          }
          auto *previous_generations =
              reinterpret_cast<std::optional<ResourceGeneration> *>(
                  base + data.activation_layout.previous_generations_offset);
          for (std::size_t resource = 0; resource < data.resource_slots.size();
               ++resource) {
            std::construct_at(previous_generations + resource, std::nullopt);
          }
          std::fill_n(reinterpret_cast<std::uint8_t *>(
                          base + data.activation_layout.changed_values_offset),
                      data.values.size(), std::uint8_t{0});
        }
      }
    } catch (...) {
      release_allocations();
      throw;
    }
  }

  ProgramRuntime(const ProgramRuntime &) = delete;
  ProgramRuntime &operator=(const ProgramRuntime &) = delete;

  ~ProgramRuntime() {
    for (ResourceId resource = 0; resource < m_resources.size(); ++resource) {
      const auto &runtime = m_resources[resource];
      for (std::uint32_t slot = 0; slot < runtime.slot_count; ++slot) {
        const auto &state = m_slots[runtime.first_slot + slot];
        if (state.constructed && runtime.destroy != nullptr) {
          runtime.destroy(pointer(resource, slot));
        }
      }
    }
    if (m_activation_storage != nullptr) {
      for (std::uint32_t activation = 0; activation < m_activation_capacity;
           ++activation) {
        auto *base = static_cast<std::byte *>(m_activation_storage) +
                     m_activation_stride * activation;
        auto *generations =
            reinterpret_cast<std::optional<ResourceGeneration> *>(
                base + m_activation_generations_offset);
        for (std::size_t resource = 0; resource < m_resources.size();
             ++resource) {
          std::destroy_at(generations + resource);
        }
        auto *previous_generations =
            reinterpret_cast<std::optional<ResourceGeneration> *>(
                base + m_activation_previous_generations_offset);
        for (std::size_t resource = 0; resource < m_resources.size();
             ++resource) {
          std::destroy_at(previous_generations + resource);
        }
      }
    }
    release_allocations();
  }

  void bind(const ProgramData &data, std::string_view name, void *pointer) {
    if (pointer == nullptr) {
      throw std::invalid_argument("external resource pointer cannot be null");
    }
    std::lock_guard lock(m_mutex);
    for (std::size_t index = 0; index < data.externals.size(); ++index) {
      if (data.externals[index].name.view() == name) {
        m_external_bindings[index] = pointer;
        return;
      }
    }
    throw std::invalid_argument("external resource name is not declared");
  }

  void *external(const ProgramData &data, ResourceId resource) const {
    std::lock_guard lock(m_mutex);
    for (std::size_t index = 0; index < data.externals.size(); ++index) {
      if (data.externals[index].resource == resource) {
        if (m_external_bindings[index] == nullptr) {
          throw std::logic_error("external resource is not bound");
        }
        return m_external_bindings[index];
      }
    }
    throw std::invalid_argument("resource is not external");
  }

  bool all_externals_bound() const {
    std::lock_guard lock(m_mutex);
    return std::all_of(m_external_bindings.begin(), m_external_bindings.end(),
                       [](void *pointer) { return pointer != nullptr; });
  }

  void release(const ProgramData &data, std::string_view name,
               std::uint64_t count) {
    if (count == 0) {
      return;
    }
    std::lock_guard lock(m_mutex);
    for (std::size_t index = 0; index < data.semaphore_names.size(); ++index) {
      if (data.semaphore_names[index].view() != name) {
        continue;
      }
      if (count > std::numeric_limits<std::uint64_t>::max() -
                      m_semaphore_tokens[index]) {
        throw std::overflow_error("Flow semaphore token count overflow");
      }
      m_semaphore_tokens[index] += count;
      return;
    }
    throw std::invalid_argument("Flow semaphore name is not declared");
  }

  bool try_consume_token(std::uint32_t semaphore) {
    std::lock_guard lock(m_mutex);
    if (semaphore >= m_semaphore_tokens.size()) {
      throw std::logic_error("compiled semaphore is out of range");
    }
    if (m_semaphore_tokens[semaphore] == 0) {
      return false;
    }
    --m_semaphore_tokens[semaphore];
    return true;
  }

  std::uint32_t begin_execution(const ProgramData &data) {
    std::unique_lock lock(m_mutex);
    std::uint32_t activation = 0;
    while (true) {
      for (; activation < m_activation_occupied.size(); ++activation) {
        if (!m_activation_occupied[activation]) {
          break;
        }
      }
      if (activation < m_activation_occupied.size()) {
        break;
      }
      m_available.wait(lock);
      activation = 0;
    }
    if (!std::all_of(m_external_bindings.begin(), m_external_bindings.end(),
                     [](void *pointer) { return pointer != nullptr; })) {
      throw std::logic_error("not all external resources are bound");
    }
    m_activation_occupied[activation] = true;
    m_activation_running[activation] = true;
    m_activation_sequences[activation] = m_next_activation++;
    auto **value_pointers = activation_values(data, activation);
    auto **arguments = activation_arguments(data, activation);
    auto *generations = activation_generations(data, activation);
    auto *previous_generations =
        activation_previous_generations(data, activation);
    auto *value_changed = activation_changed(data, activation);
    std::fill_n(value_pointers, data.values.size(), nullptr);
    std::fill_n(arguments, data.accesses.size(), nullptr);
    std::fill_n(value_changed, data.values.size(), std::uint8_t{0});
    for (NodeId node = 0; node < data.nodes.size(); ++node) {
      if (!data.nodes[node].runs_once) {
        continue;
      }
      if (m_once_claimed[node]) {
        continue;
      } else {
        m_once_claimed[node] = true;
        m_once_claim_sequences[node] = m_activation_sequences[activation];
      }
    }
    if (m_active_generations.size() != data.resource_slots.size()) {
      throw std::logic_error("runtime resource table size changed");
    }
    for (ResourceId resource = 0; resource < m_active_generations.size();
         ++resource) {
      generations[resource] = std::nullopt;
      previous_generations[resource] = std::nullopt;
      const auto generation = m_active_generations[resource];
      if (generation) {
        const auto pointer_value =
            pointer(generation->resource, generation->slot);
        for (ValueId value = 0; value < data.values.size(); ++value) {
          if (data.values[value].resource == resource) {
            value_pointers[value] = pointer_value;
          }
        }
      }
    }
    for (std::size_t external = 0; external < data.externals.size();
         ++external) {
      for (ValueId value = 0; value < data.values.size(); ++value) {
        if (data.values[value].resource == data.externals[external].resource) {
          value_pointers[value] = m_external_bindings[external];
          value_changed[value] = 1;
        }
      }
    }
    for (std::size_t cutoff = 0; cutoff < data.cutoffs.size(); ++cutoff) {
      if (m_cutoffs[cutoff].initialized) {
        value_pointers[data.cutoffs[cutoff].output] = m_cutoffs[cutoff].storage;
      }
    }
    return activation;
  }

  void resume_execution(std::uint32_t activation) {
    std::lock_guard lock(m_mutex);
    if (activation >= m_activation_occupied.size() ||
        !m_activation_occupied[activation] ||
        m_activation_running[activation]) {
      throw std::logic_error("activation cannot be resumed");
    }
    m_activation_running[activation] = true;
  }

  ResourceGeneration acquire_for_execution(const ProgramData &data,
                                           std::uint32_t activation,
                                           ResourceId resource) {
    auto generation = acquire(resource);
    const auto pointer_value = resolve(generation);
    std::lock_guard lock(m_mutex);
    if (activation >= m_activation_running.size() ||
        !m_activation_running[activation] ||
        resource >= m_active_generations.size()) {
      throw std::logic_error("activation is not active");
    }
    activation_generations(data, activation)[resource] = generation;
    auto *previous = activation_previous_generations(data, activation);
    if (resource >= data.resource_lifetimes.size()) {
      throw std::logic_error("resource lifetime is out of range");
    }
    if (m_active_generations[resource] &&
        data.resource_lifetimes[resource].destroy_at_shutdown &&
        (!data.resource_lifetimes[resource].preconstructed ||
         data.resource_lifetimes[resource].retained_by_feedback)) {
      previous[resource] = m_active_generations[resource];
    }
    m_active_generations[resource] = generation;
    auto **value_pointers = activation_values(data, activation);
    for (ValueId value = 0; value < data.values.size(); ++value) {
      if (data.values[value].resource == resource) {
        value_pointers[value] = pointer_value;
      }
    }
    return generation;
  }

  ResourceGeneration acquire_for_preconstruction(const ProgramData &data,
                                                 std::uint32_t activation,
                                                 ResourceId resource) {
    auto generation = acquire_unconstructed(resource);
    const auto pointer_value = resolve(generation);
    std::lock_guard lock(m_mutex);
    if (activation >= m_activation_running.size() ||
        !m_activation_running[activation] ||
        resource >= m_active_generations.size()) {
      throw std::logic_error("activation is not active");
    }
    activation_generations(data, activation)[resource] = generation;
    m_active_generations[resource] = generation;
    auto **value_pointers = activation_values(data, activation);
    for (ValueId value = 0; value < data.values.size(); ++value) {
      if (data.values[value].resource == resource) {
        value_pointers[value] = pointer_value;
      }
    }
    return generation;
  }

  void evaluate_cutoff(ProgramData &data, std::uint32_t activation,
                       std::size_t cutoff_index) {
    if (cutoff_index >= data.cutoffs.size() ||
        cutoff_index >= m_cutoffs.size()) {
      throw std::logic_error("compiled cutoff is out of range");
    }
    auto &compiled = data.cutoffs[cutoff_index];
    auto &runtime = m_cutoffs[cutoff_index];
    void *input = nullptr;
    {
      std::lock_guard lock(m_mutex);
      auto **values = activation_values(data, activation);
      if (compiled.input >= data.values.size() ||
          compiled.output >= data.values.size() ||
          values[compiled.input] == nullptr) {
        throw std::logic_error("cutoff input has no runtime value");
      }
      input = values[compiled.input];
    }
    const bool changed =
        !runtime.initialized || !compiled.equal(runtime.storage, input);
    if (changed) {
      if (runtime.initialized) {
        runtime.destroy(runtime.storage);
        runtime.initialized = false;
      }
      compiled.copy_construct(runtime.storage, input);
      runtime.initialized = true;
    }
    {
      std::lock_guard lock(m_mutex);
      activation_values(data, activation)[compiled.output] = runtime.storage;
      activation_changed(data, activation)[compiled.output] = changed ? 1 : 0;
    }
  }

  bool should_execute(const ProgramData &data, std::uint32_t activation,
                      NodeId node) {
    std::lock_guard lock(m_mutex);
    const auto &compiled = data.nodes[node];
    if (compiled.runs_once) {
      if (m_completed_once[node] ||
          m_once_claim_sequences[node] != m_activation_sequences[activation]) {
        return false;
      }
    }
    if (compiled.memoized && compiled.kind != OperationKind::cutoff &&
        compiled.inputs.count != 0) {
      const auto *changed = activation_changed(data, activation);
      for (std::uint32_t index = 0; index < compiled.inputs.count; ++index) {
        if (changed[data.inputs[compiled.inputs.offset + index]] != 0) {
          return true;
        }
      }
      return false;
    }
    return true;
  }

  void mark_outputs_changed(const ProgramData &data, std::uint32_t activation,
                            const CompiledNode &node) {
    std::lock_guard lock(m_mutex);
    auto *changed = activation_changed(data, activation);
    for (std::uint32_t index = 0; index < node.outputs.count; ++index) {
      changed[data.outputs[node.outputs.offset + index]] = 1;
    }
  }

  void retain_outputs(const ProgramData &data, std::uint32_t activation,
                      const CompiledNode &node) {
    std::lock_guard lock(m_mutex);
    auto **values = activation_values(data, activation);
    for (std::uint32_t index = 0; index < node.outputs.count; ++index) {
      const auto value = data.outputs[node.outputs.offset + index];
      if (value >= data.values.size()) {
        throw std::logic_error("compiled output is out of range");
      }
      const auto &compiled_value = data.values[value];
      if (compiled_value.external ||
          compiled_value.resource >= m_active_generations.size()) {
        continue;
      }
      const auto generation = m_active_generations[compiled_value.resource];
      if (generation) {
        validate(*generation);
        values[value] = pointer(generation->resource, generation->slot);
      }
    }
  }

  bool will_execute(const ProgramData &data, std::uint32_t activation,
                    NodeId node) const {
    std::lock_guard lock(m_mutex);
    return !data.nodes[node].runs_once ||
           m_once_claim_sequences[node] == m_activation_sequences[activation];
  }

  void clear_error() noexcept {
    std::lock_guard lock(m_mutex);
    m_error = nullptr;
  }

  void complete_node(const ProgramData &data, NodeId node) {
    std::lock_guard lock(m_mutex);
    if (data.nodes[node].runs_once) {
      m_completed_once[node] = true;
    }
  }

  void select_phi(const ProgramData &data, std::uint32_t activation,
                  const CompiledPhi &phi) {
    bool use_feedback = false;
    {
      std::lock_guard lock(m_mutex);
      if (activation >= m_activation_sequences.size()) {
        throw std::logic_error("activation index is out of range");
      }
      use_feedback = m_activation_sequences[activation] != 0;
    }
    select_phi(data, activation, phi, use_feedback);
  }

  void select_phi(const ProgramData &data, std::uint32_t activation,
                  const CompiledPhi &phi, bool use_feedback) {
    std::lock_guard lock(m_mutex);
    auto **values = activation_values(data, activation);
    const auto selected = use_feedback ? phi.feedback : phi.initial;
    if (selected >= data.values.size() || phi.output >= data.values.size() ||
        values[selected] == nullptr) {
      throw std::logic_error("phi input has no retained runtime value");
    }
    values[phi.output] = values[selected];
    activation_changed(data, activation)[phi.output] = 1;
  }

  void reset_control_outputs(const ProgramData &data, std::uint32_t activation,
                             span<const NodeId> nodes) {
    std::lock_guard lock(m_mutex);
    auto *changed = activation_changed(data, activation);
    for (const auto node : nodes) {
      if (node >= data.nodes.size()) {
        throw std::logic_error("structured control node is out of range");
      }
      const auto outputs = data.nodes[node].outputs;
      if (outputs.offset + outputs.count > data.outputs.size()) {
        throw std::logic_error("structured control output range is invalid");
      }
      for (std::uint32_t index = 0; index < outputs.count; ++index) {
        changed[data.outputs[outputs.offset + index]] = 0;
      }
    }
  }

  void merge_resource_changes(const ProgramData &data, std::uint32_t activation,
                              span<const ResourceId> primary,
                              span<const ResourceId> secondary) {
    std::lock_guard lock(m_mutex);
    auto *changed = activation_changed(data, activation);
    const auto merge = [&](ResourceId resource) {
      bool resource_changed = false;
      for (ValueId value = 0; value < data.values.size(); ++value) {
        if (data.values[value].resource == resource) {
          resource_changed |= changed[value] != 0;
        }
      }
      for (ValueId value = 0; value < data.values.size(); ++value) {
        if (data.values[value].resource == resource) {
          changed[value] = resource_changed ? 1 : 0;
        }
      }
    };
    for (const auto resource : primary) {
      merge(resource);
    }
    for (const auto resource : secondary) {
      if (std::find(primary.begin(), primary.end(), resource) ==
          primary.end()) {
        merge(resource);
      }
    }
  }

  bool condition(const ProgramData &data, std::uint32_t activation,
                 ValueId value) const {
    std::lock_guard lock(m_mutex);
    if (value >= data.values.size()) {
      throw std::logic_error("structured control value is out of range");
    }
    const auto resource = data.values[value].resource;
    if (resource >= data.resource_types.size() ||
        data.resource_types[resource] != type_id<bool>()) {
      throw std::logic_error("structured control value is not boolean");
    }
    auto **values = activation_values(data, activation);
    if (values[value] == nullptr) {
      throw std::logic_error("structured control value has no runtime storage");
    }
    return *static_cast<const bool *>(values[value]);
  }

  void release_previous(const ProgramData &data, std::uint32_t activation,
                        span<const ResourceId> resources) {
    Vector<ResourceGeneration, allocator_ref> releases;
    {
      std::lock_guard lock(m_mutex);
      auto *previous = activation_previous_generations(data, activation);
      for (const auto resource : resources) {
        if (resource >= data.resource_slots.size()) {
          throw std::logic_error("structured control resource is out of range");
        }
        if (previous[resource]) {
          releases.push_back(*previous[resource]);
          previous[resource] = std::nullopt;
        }
      }
    }
    for (const auto &generation : releases) {
      release(generation);
    }
  }

  void prepare_arguments(const ProgramData &data, std::uint32_t activation,
                         const CompiledNode &node) {
    std::lock_guard lock(m_mutex);
    auto **values = activation_values(data, activation);
    auto **arguments = activation_arguments(data, activation);
    if (node.accesses.offset + node.accesses.count > data.accesses.size()) {
      throw std::logic_error("compiled node accesses are out of range");
    }
    for (std::uint32_t index = 0; index < node.accesses.count; ++index) {
      const auto flat = node.accesses.offset + index;
      const auto &access = data.accesses[flat];
      const auto value =
          access.output != null_value_id ? access.output : access.input;
      if (value == null_value_id || value >= data.values.size() ||
          values[value] == nullptr) {
        throw std::logic_error("compiled access has no runtime value " +
                               std::to_string(value) + " for resource " +
                               std::to_string(access.resource));
      }
      arguments[flat] = values[value];
    }
  }

  span<void *const> arguments(const ProgramData &data, std::uint32_t activation,
                              const CompiledNode &node) {
    return {activation_arguments(data, activation) + node.accesses.offset,
            node.accesses.count};
  }

  std::optional<ResourceGeneration> active_generation(const ProgramData &data,
                                                      std::uint32_t activation,
                                                      ResourceId resource) {
    std::lock_guard lock(m_mutex);
    if (resource >= data.resource_slots.size()) {
      throw std::invalid_argument("active resource is out of range");
    }
    return activation_generations(data, activation)[resource];
  }

  void release_active(const ProgramData &data, std::uint32_t activation,
                      ResourceId resource) {
    std::optional<ResourceGeneration> generation;
    {
      std::lock_guard lock(m_mutex);
      if (resource >= data.resource_slots.size()) {
        return;
      }
      auto *generations = activation_generations(data, activation);
      generation = generations[resource];
      generations[resource] = std::nullopt;
      if (generation && m_active_generations[resource] &&
          m_active_generations[resource]->resource == generation->resource &&
          m_active_generations[resource]->slot == generation->slot &&
          m_active_generations[resource]->generation ==
              generation->generation) {
        m_active_generations[resource] = std::nullopt;
      }
    }
    if (generation) {
      release(*generation);
    }
  }

  void record_error(std::exception_ptr error) noexcept {
    std::lock_guard lock(m_mutex);
    if (m_error == nullptr) {
      m_error = std::move(error);
    }
  }

  bool failed() const noexcept {
    std::lock_guard lock(m_mutex);
    return m_error != nullptr;
  }

  void finish_execution(const ProgramData &data, std::uint32_t activation,
                        bool activation_complete) {
    Vector<ResourceGeneration, allocator_ref> releases;
    {
      std::lock_guard lock(m_mutex);
      if (activation_complete) {
        auto *previous_generations =
            activation_previous_generations(data, activation);
        for (ResourceId resource = 0; resource < data.resource_slots.size();
             ++resource) {
          auto &generation = previous_generations[resource];
          if (generation) {
            releases.push_back(*generation);
            generation = std::nullopt;
          }
        }
        m_activation_occupied[activation] = false;
      }
      m_activation_running[activation] = false;
    }
    for (const auto &generation : releases) {
      release(generation);
    }
    m_available.notify_all();
  }

  void rethrow_error() const {
    std::lock_guard lock(m_mutex);
    if (m_error != nullptr) {
      std::rethrow_exception(m_error);
    }
  }

  std::optional<ResourceGeneration> try_acquire(ResourceId resource) {
    std::lock_guard lock(m_mutex);
    return try_acquire_locked(resource);
  }

  ResourceGeneration acquire(ResourceId resource) {
    std::unique_lock lock(m_mutex);
    while (true) {
      if (auto result = try_acquire_locked(resource)) {
        return *result;
      }
      m_available.wait(lock);
    }
  }

  ResourceGeneration acquire_unconstructed(ResourceId resource) {
    std::unique_lock lock(m_mutex);
    while (true) {
      if (resource >= m_resources.size() ||
          m_resources[resource].slot_count == 0 ||
          !m_resources[resource].preconstructed) {
        throw std::invalid_argument(
            "resource has no preconstructed storage slots");
      }
      auto &runtime = m_resources[resource];
      for (std::uint32_t slot = 0; slot < runtime.slot_count; ++slot) {
        auto &state = m_slots[runtime.first_slot + slot];
        if (state.occupied || state.constructed ||
            physical_range_occupied(resource, slot)) {
          continue;
        }
        state.occupied = true;
        state.generation = runtime.next_generation++;
        if (runtime.next_generation == 0) {
          runtime.next_generation = 1;
        }
        return ResourceGeneration{
            .resource = resource,
            .slot = slot,
            .generation = state.generation,
        };
      }
      m_available.wait(lock);
    }
  }

  void *resolve(const ResourceGeneration &generation) const {
    std::lock_guard lock(m_mutex);
    validate(generation);
    return pointer(generation.resource, generation.slot);
  }

  void mark_constructed(const ResourceGeneration &generation) {
    std::lock_guard lock(m_mutex);
    auto &slot = validate(generation);
    if (slot.constructed) {
      throw std::logic_error("resource generation is already constructed");
    }
    slot.constructed = true;
  }

  void release(const ResourceGeneration &generation) {
    void (*destroy)(void *) noexcept = nullptr;
    void *storage = nullptr;
    {
      std::lock_guard lock(m_mutex);
      auto &slot = validate(generation);
      const auto &resource = m_resources[generation.resource];
      if (slot.constructed && !resource.preconstructed) {
        destroy = resource.destroy;
        storage = pointer(generation.resource, generation.slot);
        slot.constructed = false;
      }
    }
    if (destroy != nullptr) {
      destroy(storage);
    }
    {
      std::lock_guard lock(m_mutex);
      auto &slot = validate(generation);
      slot.occupied = false;
    }
    m_available.notify_one();
  }

private:
  std::byte *activation_base(const ProgramData &data,
                             std::uint32_t activation) const {
    if (activation >= data.activation_layout.capacity ||
        m_activation_storage == nullptr) {
      throw std::logic_error("activation state is out of range");
    }
    return static_cast<std::byte *>(m_activation_storage) +
           data.activation_layout.stride * activation;
  }

  void **activation_values(const ProgramData &data,
                           std::uint32_t activation) const {
    return reinterpret_cast<void **>(activation_base(data, activation) +
                                     data.activation_layout.values_offset);
  }

  void **activation_arguments(const ProgramData &data,
                              std::uint32_t activation) const {
    return reinterpret_cast<void **>(activation_base(data, activation) +
                                     data.activation_layout.arguments_offset);
  }

  std::optional<ResourceGeneration> *
  activation_generations(const ProgramData &data,
                         std::uint32_t activation) const {
    return reinterpret_cast<std::optional<ResourceGeneration> *>(
        activation_base(data, activation) +
        data.activation_layout.generations_offset);
  }

  std::optional<ResourceGeneration> *
  activation_previous_generations(const ProgramData &data,
                                  std::uint32_t activation) const {
    return reinterpret_cast<std::optional<ResourceGeneration> *>(
        activation_base(data, activation) +
        data.activation_layout.previous_generations_offset);
  }

  std::uint8_t *activation_changed(const ProgramData &data,
                                   std::uint32_t activation) const {
    return reinterpret_cast<std::uint8_t *>(
        activation_base(data, activation) +
        data.activation_layout.changed_values_offset);
  }

  void release_allocations() noexcept {
    for (auto &cutoff : m_cutoffs) {
      if (cutoff.initialized) {
        cutoff.destroy(cutoff.storage);
      }
      allocator_ref{}.deallocate(cutoff.storage, cutoff.allocation_size,
                                 cutoff.allocation_alignment);
      cutoff.storage = nullptr;
      cutoff.initialized = false;
    }
    if (m_storage != nullptr) {
      allocator_ref{}.deallocate(m_storage, m_storage_size,
                                 m_storage_alignment);
      m_storage = nullptr;
    }
    if (m_activation_storage != nullptr) {
      allocator_ref{}.deallocate(m_activation_storage,
                                 m_activation_storage_size,
                                 m_activation_storage_alignment);
      m_activation_storage = nullptr;
    }
  }

  std::optional<ResourceGeneration> try_acquire_locked(ResourceId resource) {
    if (resource >= m_resources.size() ||
        m_resources[resource].slot_count == 0) {
      throw std::invalid_argument("resource has no internal storage slots");
    }
    auto &runtime = m_resources[resource];
    if (runtime.preconstructed) {
      for (std::uint32_t slot = 0; slot < runtime.slot_count; ++slot) {
        if (!m_slots[runtime.first_slot + slot].constructed) {
          return std::nullopt;
        }
      }
    }
    for (std::uint32_t slot = 0; slot < runtime.slot_count; ++slot) {
      auto &state = m_slots[runtime.first_slot + slot];
      if (state.occupied || (runtime.preconstructed && !state.constructed) ||
          physical_range_occupied(resource, slot)) {
        continue;
      }
      state.occupied = true;
      if (!runtime.preconstructed) {
        state.constructed = false;
      }
      state.generation = runtime.next_generation++;
      if (runtime.next_generation == 0) {
        runtime.next_generation = 1;
      }
      return ResourceGeneration{
          .resource = resource,
          .slot = slot,
          .generation = state.generation,
      };
    }
    return std::nullopt;
  }

  bool physical_range_occupied(ResourceId resource, std::uint32_t slot) const {
    const auto &candidate = m_resources[resource];
    const auto candidate_begin = candidate.offset + candidate.stride * slot;
    const auto candidate_end = candidate_begin + candidate.stride;
    for (ResourceId other = 0; other < m_resources.size(); ++other) {
      const auto &other_resource = m_resources[other];
      for (std::uint32_t other_slot = 0; other_slot < other_resource.slot_count;
           ++other_slot) {
        if (!m_slots[other_resource.first_slot + other_slot].occupied) {
          continue;
        }
        const auto other_begin =
            other_resource.offset + other_resource.stride * other_slot;
        const auto other_end = other_begin + other_resource.stride;
        if (candidate_begin < other_end && other_begin < candidate_end) {
          return true;
        }
      }
    }
    return false;
  }

  RuntimeSlot &validate(const ResourceGeneration &generation) {
    if (generation.resource >= m_resources.size()) {
      throw std::invalid_argument("resource generation is out of range");
    }
    const auto &resource = m_resources[generation.resource];
    if (generation.slot >= resource.slot_count) {
      throw std::invalid_argument("resource generation slot is out of range");
    }
    auto &slot = m_slots[resource.first_slot + generation.slot];
    if (!slot.occupied || slot.generation != generation.generation) {
      throw std::logic_error("resource generation is no longer live");
    }
    return slot;
  }

  const RuntimeSlot &validate(const ResourceGeneration &generation) const {
    if (generation.resource >= m_resources.size()) {
      throw std::invalid_argument("resource generation is out of range");
    }
    const auto &resource = m_resources[generation.resource];
    if (generation.slot >= resource.slot_count) {
      throw std::invalid_argument("resource generation slot is out of range");
    }
    const auto &slot = m_slots[resource.first_slot + generation.slot];
    if (!slot.occupied || slot.generation != generation.generation) {
      throw std::logic_error("resource generation is no longer live");
    }
    return slot;
  }

  void *pointer(ResourceId resource, std::uint32_t slot) const {
    const auto &runtime = m_resources[resource];
    return static_cast<std::byte *>(m_storage) + runtime.offset +
           runtime.stride * slot;
  }

  void *m_storage = nullptr;
  std::size_t m_storage_size = 0;
  std::size_t m_storage_alignment = 1;
  void *m_activation_storage = nullptr;
  std::size_t m_activation_storage_size = 0;
  std::size_t m_activation_storage_alignment = 1;
  std::uint32_t m_activation_capacity = 0;
  std::size_t m_activation_stride = 0;
  std::size_t m_activation_generations_offset = 0;
  std::size_t m_activation_previous_generations_offset = 0;
  Vector<RuntimeResource, allocator_ref> m_resources;
  Vector<RuntimeSlot, allocator_ref> m_slots;
  Vector<void *, allocator_ref> m_external_bindings;
  Vector<std::uint64_t, allocator_ref> m_semaphore_tokens;
  Vector<std::optional<ResourceGeneration>, allocator_ref> m_active_generations;
  Vector<bool, allocator_ref> m_completed_once;
  Vector<bool, allocator_ref> m_once_claimed;
  Vector<std::uint64_t, allocator_ref> m_once_claim_sequences;
  Vector<bool, allocator_ref> m_activation_occupied;
  Vector<bool, allocator_ref> m_activation_running;
  Vector<std::uint64_t, allocator_ref> m_activation_sequences;
  Vector<RuntimeCutoff, allocator_ref> m_cutoffs;
  std::exception_ptr m_error;
  std::uint64_t m_next_activation = 0;
  mutable std::mutex m_mutex;
  std::condition_variable m_available;
};

} // namespace strobe::flow::detail
