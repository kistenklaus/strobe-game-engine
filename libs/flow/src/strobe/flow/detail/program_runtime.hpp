#pragma once

#include <strobe/flow/program.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstring>
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
  static constexpr std::uint32_t activation_pool_capacity =
      sync::JobScheduler::SLOT_COUNT + 2;

  /// called by submitting thread
  explicit ProgramRuntime(const ProgramData &data)
      : m_external_bindings(data.externals.size(), nullptr, allocator_ref{}),
        m_semaphore_tokens(data.semaphore_names.size(), 0, allocator_ref{}) {
    m_activation_generations_offset = data.activation_layout.generations_offset;
    m_activation_previous_generations_offset =
        data.activation_layout.previous_generations_offset;
    m_resources.reserve(data.resources.size());
    std::uint32_t first_slot = 0;
    for (ResourceId resource = 0; resource < data.resources.size();
         ++resource) {
      const auto &layout = data.resources[resource].slots;
      auto destroy = static_cast<void (*)(void *) noexcept>(nullptr);
      if (resource < data.resources.size()) {
        destroy = data.resources[resource].lifetime.destroy;
      }
      m_resources.push_back(RuntimeResource{
          .offset = layout.offset,
          .stride = layout.slot_stride,
          .first_slot = first_slot,
          .slot_count = layout.slot_count,
          .next_generation = 1,
          .destroy = destroy,
          .preconstructed = resource < data.resources.size() &&
                            data.resources[resource].lifetime.preconstructed,
      });
      if (layout.slot_count >
          std::numeric_limits<std::uint32_t>::max() - first_slot) {
        throw std::length_error("Flow runtime slot table overflow");
      }
      first_slot += layout.slot_count;
    }
    m_slots.resize(first_slot);
    m_values_by_resource.resize(data.resources.size());
    for (ValueId value = 0; value < data.values.size(); ++value) {
      const auto resource = data.values[value].resource;
      assert(resource < m_values_by_resource.size());
      m_values_by_resource[resource].push_back(value);
    }
    m_projection_by_resource.resize(data.resources.size(), null_node_id);
    for (std::uint32_t projection = 0; projection < data.projections.size();
         ++projection) {
      const auto resource = data.projections[projection].resource;
      assert(resource < m_projection_by_resource.size());
      assert(m_projection_by_resource[resource] == null_node_id);
      m_projection_by_resource[resource] = projection;
    }
    m_cutoff_by_value.resize(data.values.size(), null_node_id);
    for (std::uint32_t cutoff = 0; cutoff < data.cutoffs.size(); ++cutoff) {
      const auto output = data.cutoffs[cutoff].output;
      assert(output < m_cutoff_by_value.size());
      assert(m_cutoff_by_value[output] == null_node_id);
      m_cutoff_by_value[output] = cutoff;
    }
    assert(data.physical_storage.slot_physical_ids.size() == first_slot);
    m_slot_physical_ids = data.physical_storage.slot_physical_ids;
    assert(!data.physical_storage.frontier_offsets.empty());
    m_physical_occupancy.resize(
        data.physical_storage.frontier_offsets.size() - 1, 0);
    m_active_generations.resize(data.resources.size(), std::nullopt);
    m_feedback_generations.resize(data.phis.size(), std::nullopt);
    m_feedback_phi_is_external.resize(data.phis.size(), false);
    m_feedback_phis_by_value.resize(data.values.size());
    for (std::uint32_t phi = 0; phi < data.phis.size(); ++phi) {
      const auto feedback = data.phis[phi].feedback;
      assert(feedback < m_feedback_phis_by_value.size());
      m_feedback_phis_by_value[feedback].push_back(phi);
    }
    for (const auto &region : data.feedback_regions) {
      for (const auto phi : data.phis_of(region)) {
        assert(phi < m_feedback_phi_is_external.size());
        m_feedback_phi_is_external[phi] = true;
      }
    }
    m_activation_occupied.resize(activation_pool_capacity, false);
    m_activation_running.resize(activation_pool_capacity, false);
    m_activation_sequences.resize(activation_pool_capacity, 0);

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
      if (data.activation_layout.stride != 0) {
        m_activation_block_size = data.activation_layout.stride;
        m_activation_storage_alignment =
            std::max(data.activation_layout.alignment, alignof(void *));
      }
      if (!data.immediate_data.empty()) {
        m_immediate_storage_alignment =
            std::max(data.immediate_alignment, alignof(void *));
        m_immediate_storage_size = data.immediate_data.size();
        const auto remainder =
            m_immediate_storage_size % m_immediate_storage_alignment;
        if (remainder != 0) {
          if (m_immediate_storage_size >
              std::numeric_limits<std::size_t>::max() -
                  (m_immediate_storage_alignment - remainder)) {
            throw std::length_error("Flow immediate storage size overflow");
          }
          m_immediate_storage_size += m_immediate_storage_alignment - remainder;
        }
        m_immediate_storage = allocator_ref{}.allocate(
            m_immediate_storage_size, m_immediate_storage_alignment);
        if (m_immediate_storage == nullptr) {
          throw std::bad_alloc();
        }
        std::memcpy(m_immediate_storage, data.immediate_data.data(),
                    data.immediate_data.size());
      }
      m_immediate_pointers.reserve(data.immediates.size());
      m_immediate_counts.reserve(data.immediates.size());
      for (const auto &immediate : data.immediates) {
        m_immediate_pointers.push_back(
            m_immediate_storage == nullptr
                ? nullptr
                : static_cast<std::byte *>(m_immediate_storage) +
                      immediate.offset);
        m_immediate_counts.push_back(immediate.element_count);
      }
    } catch (...) {
      release_allocations();
      throw;
    }
  }

  /// called by submitting thread
  ProgramRuntime(const ProgramRuntime &) = delete;
  /// called by submitting thread
  ProgramRuntime &operator=(const ProgramRuntime &) = delete;

  /// called by submitting thread
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
    for (std::uint32_t activation = 0; activation < m_activation_block_count;
         ++activation) {
      auto *base = static_cast<std::byte *>(m_activation_blocks[activation]);
      if (base != nullptr) {
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

  /// called by submitting thread
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

  /// called by submitting thread
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

  /// called by submitting thread
  bool all_externals_bound() const {
    std::lock_guard lock(m_mutex);
    return std::all_of(m_external_bindings.begin(), m_external_bindings.end(),
                       [](void *pointer) { return pointer != nullptr; });
  }

  /// called by submitting thread
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

  /// called by submitting thread
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

  /// called by submitting thread
  bool try_consume_tokens(const ProgramData &data,
                          const CompiledFeedbackRegion &region) {
    std::lock_guard lock(m_mutex);
    if (region.token_requirements.offset + region.token_requirements.count >
        data.feedback_token_requirements.size()) {
      throw std::logic_error("feedback token requirement range is invalid");
    }
    const auto requirements = data.token_requirements_of(region);
    for (const auto &requirement : requirements) {
      if (requirement.semaphore >= m_semaphore_tokens.size()) {
        throw std::logic_error("feedback token requirement is invalid");
      }
      if (m_semaphore_tokens[requirement.semaphore] < requirement.count) {
        return false;
      }
    }
    for (const auto &requirement : requirements) {
      m_semaphore_tokens[requirement.semaphore] -= requirement.count;
    }
    return true;
  }

  /// called by submitting thread
  std::uint32_t begin_execution(const ProgramData &data) {
    std::unique_lock lock(m_mutex);
    if (!std::all_of(m_external_bindings.begin(), m_external_bindings.end(),
                     [](void *pointer) { return pointer != nullptr; })) {
      throw std::logic_error("not all external resources are bound");
    }
    if (m_free_activation_count == 0) {
      if (m_activation_block_count >= activation_pool_capacity) {
        m_available.wait(lock, [&] { return m_free_activation_count != 0; });
      } else {
        allocate_activation(data);
      }
    }
    const auto activation = m_free_activations[--m_free_activation_count];
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
    std::fill_n(arguments, data.node_storage.accesses.size(), nullptr);
    std::fill_n(value_changed, data.values.size(), std::uint8_t{0});
    if (m_active_generations.size() != data.resources.size()) {
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
        for (const auto value : m_values_by_resource[resource]) {
          value_pointers[value] = pointer_value;
        }
      }
    }
    for (std::size_t external = 0; external < data.externals.size();
         ++external) {
      for (const auto value :
           m_values_by_resource[data.externals[external].resource]) {
        value_pointers[value] = m_external_bindings[external];
        value_changed[value] = 1;
      }
    }
    for (std::size_t cutoff = 0; cutoff < data.cutoffs.size(); ++cutoff) {
      if (m_cutoffs[cutoff].initialized) {
        value_pointers[data.cutoffs[cutoff].output] = m_cutoffs[cutoff].storage;
      }
    }
    return activation;
  }

  /// called by submitting thread
  void resume_execution(std::uint32_t activation) {
    std::lock_guard lock(m_mutex);
    if (activation >= m_activation_occupied.size() ||
        !m_activation_occupied[activation] ||
        m_activation_running[activation]) {
      throw std::logic_error("activation cannot be resumed");
    }
    m_activation_running[activation] = true;
  }

  /// called by submitting thread
  /// called by worker thread
  ResourceGeneration
  acquire_for_execution(const ProgramData &data, std::uint32_t activation,
                        ResourceId resource,
                        std::uint32_t preferred_slot = null_node_id) {
    ResourceGeneration generation{};
    if (preferred_slot == null_node_id) {
      generation = acquire(resource);
    } else {
      {
        std::lock_guard lock(m_mutex);
        const auto reserved = try_acquire_locked(resource, preferred_slot);
        assert(reserved.has_value());
        generation = *reserved;
      }
    }
    const auto pointer_value = pointer(generation.resource, generation.slot);
    std::lock_guard lock(m_mutex);
    assert(activation < m_activation_running.size());
    assert(m_activation_running[activation]);
    assert(resource < m_active_generations.size());
    activation_generations(data, activation)[resource] = generation;
    auto *previous = activation_previous_generations(data, activation);
    assert(resource < data.resources.size());
    if (!previous[resource] && m_active_generations[resource] &&
        data.resources[resource].lifetime.destroy_at_shutdown &&
        (!data.resources[resource].lifetime.preconstructed ||
         data.resources[resource].lifetime.retained_by_feedback)) {
      previous[resource] = m_active_generations[resource];
    }
    m_active_generations[resource] = generation;
    auto **value_pointers = activation_values(data, activation);
    for (const auto value : m_values_by_resource[resource]) {
      value_pointers[value] = pointer_value;
    }
    return generation;
  }

  /// called by worker thread
  ResourceGeneration acquire_for_preconstruction(const ProgramData &data,
                                                 std::uint32_t activation,
                                                 ResourceId resource) {
    auto generation = acquire_unconstructed(resource);
    const auto pointer_value = pointer(generation.resource, generation.slot);
    std::lock_guard lock(m_mutex);
    assert(activation < m_activation_running.size());
    assert(m_activation_running[activation]);
    assert(resource < m_active_generations.size());
    activation_generations(data, activation)[resource] = generation;
    m_active_generations[resource] = generation;
    auto **value_pointers = activation_values(data, activation);
    for (const auto value : m_values_by_resource[resource]) {
      value_pointers[value] = pointer_value;
    }
    return generation;
  }

  /// called by worker thread
  void evaluate_cutoff(ProgramData &data, std::uint32_t activation,
                       std::size_t cutoff_index) {
    assert(cutoff_index < data.cutoffs.size());
    assert(cutoff_index < m_cutoffs.size());
    auto &compiled = data.cutoffs[cutoff_index];
    auto &runtime = m_cutoffs[cutoff_index];
    void *input = nullptr;
    {
      std::lock_guard lock(m_mutex);
      auto **values = activation_values(data, activation);
      assert(compiled.input < data.values.size());
      assert(compiled.output < data.values.size());
      assert(values[compiled.input] != nullptr);
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

  /// called by worker thread
  /// per-node hot path
  bool should_execute(const ProgramData &data, std::uint32_t activation,
                      NodeId node) {
    std::lock_guard lock(m_mutex);
    const auto &compiled = data.nodes[node];
    if (compiled.memoized && compiled.kind != OperationKind::cutoff &&
        compiled.inputs.count != 0) {
      const auto *changed = activation_changed(data, activation);
      for (const auto input : data.inputs_of(compiled)) {
        if (changed[input] != 0) {
          return true;
        }
      }
      return false;
    }
    return true;
  }

  /// called by worker thread
  /// per-node hot path
  void mark_outputs_changed(const ProgramData &data, std::uint32_t activation,
                            const CompiledNode &node) {
    if (node.outputs.count == 0) {
      return;
    }
    std::lock_guard lock(m_mutex);
    auto *changed = activation_changed(data, activation);
    for (const auto output : data.outputs_of(node)) {
      changed[output] = 1;
    }
  }

  /// called by worker thread
  /// per-node hot path
  void retain_feedback_outputs(const ProgramData &data,
                               std::uint32_t activation,
                               const CompiledNode &node) {
    assert(node.outputs.offset <= data.node_storage.outputs.size());
    assert(node.outputs.count <=
           data.node_storage.outputs.size() - node.outputs.offset);
    const auto retains_feedback = std::any_of(
        data.outputs_of(node).begin(), data.outputs_of(node).end(),
        [&](ValueId output) {
          assert(output < m_feedback_phis_by_value.size());
          return std::any_of(m_feedback_phis_by_value[output].begin(),
                             m_feedback_phis_by_value[output].end(),
                             [&](std::uint32_t phi) {
                               return m_feedback_phi_is_external[phi];
                             });
        });
    if (!retains_feedback) {
      return;
    }
    std::lock_guard lock(m_mutex);
    auto *generations = activation_generations(data, activation);
    for (const auto output : data.outputs_of(node)) {
      assert(output < m_feedback_phis_by_value.size());
      for (const auto phi_index : m_feedback_phis_by_value[output]) {
        if (!m_feedback_phi_is_external[phi_index]) {
          continue;
        }
        const auto resource = data.values[output].resource;
        assert(resource < data.resources.size());
        const auto generation = generations[resource]
                                    ? generations[resource]
                                    : m_active_generations[resource];
        assert(generation.has_value());
        m_feedback_generations[phi_index] = generation;
      }
    }
  }

  /// called by worker thread
  void retain_outputs(const ProgramData &data, std::uint32_t activation,
                      const CompiledNode &node) {
    if (node.outputs.count == 0) {
      return;
    }
    std::lock_guard lock(m_mutex);
    auto **values = activation_values(data, activation);
    for (const auto value : data.outputs_of(node)) {
      assert(value < data.values.size());
      const auto &compiled_value = data.values[value];
      if (compiled_value.external ||
          compiled_value.resource >= m_active_generations.size()) {
        continue;
      }
      const auto generation = m_active_generations[compiled_value.resource];
      if (generation) {
        values[value] = pointer(generation->resource, generation->slot);
      }
    }
  }

  /// called by submitting thread
  void clear_error() noexcept {
    std::lock_guard lock(m_mutex);
    m_error = nullptr;
    m_failed.store(false, std::memory_order_release);
  }

  /// called by submitting thread
  /// called by worker thread
  void select_phi(const ProgramData &data, std::uint32_t activation,
                  const CompiledPhi &phi) {
    bool use_feedback = false;
    {
      std::lock_guard lock(m_mutex);
      assert(activation < m_activation_sequences.size());
      use_feedback = m_activation_sequences[activation] != 0;
    }
    select_phi(data, activation, phi, use_feedback);
  }

  /// called by submitting thread
  /// called by worker thread
  void select_phi(const ProgramData &data, std::uint32_t activation,
                  const CompiledPhi &phi, bool use_feedback) {
    std::lock_guard lock(m_mutex);
    auto **values = activation_values(data, activation);
    const auto selected = use_feedback ? phi.feedback : phi.initial;
    assert(selected < data.values.size());
    assert(phi.output < data.values.size());
    const auto resource = data.values[selected].resource;
    auto *previous = activation_previous_generations(data, activation);
    std::optional<ResourceGeneration> generation;
    if (use_feedback) {
      const auto phi_index = static_cast<std::size_t>(&phi - data.phis.data());
      assert(phi_index < m_feedback_generations.size());
      if (m_feedback_phi_is_external[phi_index]) {
        generation = m_feedback_generations[phi_index];
        if (generation && resource < data.resources.size()) {
          previous[resource] = generation;
        }
      } else if (resource < data.resources.size() && previous[resource]) {
        generation = previous[resource];
      } else if (resource < m_active_generations.size()) {
        generation = m_active_generations[resource];
      }
    } else if (resource < m_active_generations.size()) {
      generation = m_active_generations[resource];
    }
    auto *selected_value = values[selected];
    if ((use_feedback || selected_value == nullptr) && generation) {
      selected_value = pointer(generation->resource, generation->slot);
    }
    assert(selected_value != nullptr);
    values[phi.output] = selected_value;
    activation_changed(data, activation)[phi.output] = 1;
  }

  /// called by worker thread
  void reset_control_outputs(const ProgramData &data, std::uint32_t activation,
                             span<const NodeId> nodes) {
    std::lock_guard lock(m_mutex);
    auto *changed = activation_changed(data, activation);
    for (const auto node : nodes) {
      assert(node < data.nodes.size());
      const auto outputs = data.nodes[node].outputs;
      assert(outputs.offset <= data.node_storage.outputs.size());
      assert(outputs.count <=
             data.node_storage.outputs.size() - outputs.offset);
      for (std::uint32_t index = 0; index < outputs.count; ++index) {
        changed[data.node_storage.outputs[outputs.offset + index]] = 0;
      }
    }
  }

  /// called by worker thread
  void merge_resource_changes(const ProgramData &data, std::uint32_t activation,
                              span<const ResourceId> primary,
                              span<const ResourceId> secondary) {
    std::lock_guard lock(m_mutex);
    auto *changed = activation_changed(data, activation);
    const auto merge = [&](ResourceId resource) {
      bool resource_changed = false;
      assert(resource < m_values_by_resource.size());
      for (const auto value : m_values_by_resource[resource]) {
        resource_changed |= changed[value] != 0;
      }
      for (const auto value : m_values_by_resource[resource]) {
        changed[value] = resource_changed ? 1 : 0;
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

  /// called by worker thread
  bool condition(const ProgramData &data, std::uint32_t activation,
                 ValueId value) const {
    std::lock_guard lock(m_mutex);
    assert(value < data.values.size());
    const auto resource = data.values[value].resource;
    assert(resource < data.resources.size());
    assert(data.resources[resource].type == type_id<bool>());
    auto **values = activation_values(data, activation);
    assert(values[value] != nullptr);
    return *static_cast<const bool *>(values[value]);
  }

  /// called by worker thread
  void release_previous(const ProgramData &data, std::uint32_t activation,
                        span<const ResourceId> resources) {
    Vector<ResourceGeneration, allocator_ref> releases;
    {
      std::lock_guard lock(m_mutex);
      auto *previous = activation_previous_generations(data, activation);
      for (const auto resource : resources) {
        assert(resource < data.resources.size());
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

  /// called by worker thread
  /// per-callback hot path
  void prepare_arguments(const ProgramData &data, std::uint32_t activation,
                         const CompiledNode &node) {
    if (node.argument_count == 0) {
      return;
    }
    std::lock_guard lock(m_mutex);
    auto **values = activation_values(data, activation);
    auto **arguments = activation_arguments(data, activation);
    assert(node.accesses.offset <= data.node_storage.accesses.size());
    assert(node.accesses.count <=
           data.node_storage.accesses.size() - node.accesses.offset);
    assert(node.argument_count <= node.accesses.count);
    const auto accesses = data.accesses_of(node);
    for (std::uint32_t index = 0; index < node.argument_count; ++index) {
      const auto flat = node.accesses.offset + index;
      const auto &access = accesses[index];
      const auto value =
          access.output != null_value_id ? access.output : access.input;
      assert(value != null_value_id);
      assert(value < data.values.size());
      const auto resource = data.values[value].resource;
      if (values[value] == nullptr) {
        assert(resource < m_projection_by_resource.size());
        const auto projection_index = m_projection_by_resource[resource];
        if (projection_index != null_node_id) {
          const auto &projection = data.projections[projection_index];
          void *owner = nullptr;
          assert(projection.owner < m_values_by_resource.size());
          for (const auto owner_value :
               m_values_by_resource[projection.owner]) {
            if (values[owner_value] != nullptr) {
              owner = values[owner_value];
              break;
            }
          }
          if (owner == nullptr &&
              projection.owner < m_active_generations.size() &&
              m_active_generations[projection.owner]) {
            const auto &generation = *m_active_generations[projection.owner];
            owner = pointer(generation.resource, generation.slot);
          }
          if (owner != nullptr) {
            values[value] = projection.resolve(owner);
          }
        }
      }
      if (values[value] == nullptr && resource < m_active_generations.size() &&
          m_active_generations[resource]) {
        const auto &generation = *m_active_generations[resource];
        values[value] = pointer(generation.resource, generation.slot);
      }
      if (values[value] == nullptr) {
        assert(value < m_cutoff_by_value.size());
        const auto cutoff = m_cutoff_by_value[value];
        if (cutoff != null_node_id && m_cutoffs[cutoff].initialized) {
          values[value] = m_cutoffs[cutoff].storage;
        }
      }
      assert(values[value] != nullptr);
      arguments[flat] = values[value];
    }
  }

  /// called by worker thread
  /// per-callback hot path
  span<void *const> arguments(const ProgramData &data, std::uint32_t activation,
                              const CompiledNode &node) {
    return {activation_arguments(data, activation) + node.accesses.offset,
            node.argument_count};
  }

  /// called by worker thread
  /// per-callback hot path
  span<void *const> immediates(const CompiledCallback &callback) {
    if (callback.immediates.count == 0) {
      return {};
    }
    return {m_immediate_pointers.data() + callback.immediates.offset,
            callback.immediates.count};
  }

  /// called by worker thread
  /// per-callback hot path
  span<const std::uint32_t> immediate_counts(const CompiledCallback &callback) {
    if (callback.immediates.count == 0) {
      return {};
    }
    return {m_immediate_counts.data() + callback.immediates.offset,
            callback.immediates.count};
  }

  /// called by worker thread
  std::optional<ResourceGeneration> active_generation(const ProgramData &data,
                                                      std::uint32_t activation,
                                                      ResourceId resource) {
    std::lock_guard lock(m_mutex);
    assert(resource < data.resources.size());
    return activation_generations(data, activation)[resource];
  }

  /// called by worker thread
  void release_active(const ProgramData &data, std::uint32_t activation,
                      ResourceId resource) {
    std::optional<ResourceGeneration> generation;
    {
      std::lock_guard lock(m_mutex);
      assert(resource < data.resources.size());
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

  /// called by worker thread
  void record_error(std::exception_ptr error) noexcept {
    std::lock_guard lock(m_mutex);
    if (m_error == nullptr) {
      m_error = std::move(error);
      m_failed.store(true, std::memory_order_release);
    }
  }

  /// called by worker thread
  /// per-job hot path
  bool failed() const noexcept {
    return m_failed.load(std::memory_order_acquire);
  }

  /// called by worker thread
  void finish_execution(const ProgramData &data, std::uint32_t activation,
                        bool activation_complete) {
    Vector<ResourceGeneration, allocator_ref> releases;
    {
      std::lock_guard lock(m_mutex);
      if (activation_complete) {
        auto *previous_generations =
            activation_previous_generations(data, activation);
        for (ResourceId resource = 0; resource < data.resources.size();
             ++resource) {
          auto &generation = previous_generations[resource];
          if (generation) {
            releases.push_back(*generation);
            generation = std::nullopt;
          }
        }
        m_activation_occupied[activation] = false;
        m_free_activations[m_free_activation_count++] = activation;
      }
      m_activation_running[activation] = false;
    }
    for (const auto &generation : releases) {
      release(generation);
    }
    m_available.notify_all();
  }

  /// called by submitting thread
  void rethrow_error() const {
    std::lock_guard lock(m_mutex);
    if (m_error != nullptr) {
      std::rethrow_exception(m_error);
    }
  }

  /// called by submitting thread
  std::optional<ResourceGeneration> try_acquire(ResourceId resource) {
    std::lock_guard lock(m_mutex);
    return try_acquire_locked(resource);
  }

  /// called by submitting thread
  /// called by worker thread
  ResourceGeneration acquire(ResourceId resource) {
    std::unique_lock lock(m_mutex);
    while (true) {
      if (auto result = try_acquire_locked(resource)) {
        return *result;
      }
      m_available.wait(lock);
    }
  }

  /// called by worker thread
  ResourceGeneration acquire_unconstructed(ResourceId resource) {
    std::unique_lock lock(m_mutex);
    assert(resource < m_resources.size());
    assert(m_resources[resource].slot_count != 0);
    assert(m_resources[resource].preconstructed);
    while (true) {
      auto &runtime = m_resources[resource];
      for (std::uint32_t slot = 0; slot < runtime.slot_count; ++slot) {
        auto &state = m_slots[runtime.first_slot + slot];
        if (state.occupied || state.constructed ||
            physical_range_occupied(resource, slot)) {
          continue;
        }
        state.occupied = true;
        ++m_physical_occupancy[m_slot_physical_ids[runtime.first_slot + slot]];
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

  /// called by submitting thread
  /// called by worker thread
  void *resolve(const ResourceGeneration &generation) const {
    std::lock_guard lock(m_mutex);
    validate(generation);
    return pointer(generation.resource, generation.slot);
  }

  /// called by submitting thread
  /// called by worker thread
  void mark_constructed(const ResourceGeneration &generation) {
    std::lock_guard lock(m_mutex);
    auto &slot = validate(generation);
    assert(!slot.constructed);
    slot.constructed = true;
  }

  /// called by submitting thread
  /// called by worker thread
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
      const auto logical_slot =
          m_resources[generation.resource].first_slot + generation.slot;
      const auto physical_slot = m_slot_physical_ids[logical_slot];
      assert(m_physical_occupancy[physical_slot] != 0);
      --m_physical_occupancy[physical_slot];
      slot.occupied = false;
    }
    m_available.notify_one();
  }

private:
  /// called by submitting thread
  /// called by worker thread
  /// per-node hot path
  std::byte *activation_base(const ProgramData &,
                             std::uint32_t activation) const {
    assert(activation < m_activation_block_count);
    assert(m_activation_blocks[activation] != nullptr);
    return static_cast<std::byte *>(m_activation_blocks[activation]);
  }

  /// called by submitting thread
  void allocate_activation(const ProgramData &data) {
    if (m_activation_block_count >= activation_pool_capacity ||
        m_activation_block_size == 0) {
      throw std::length_error("Flow activation pool capacity exhausted");
    }
    auto *base = static_cast<std::byte *>(allocator_ref{}.allocate(
        m_activation_block_size, m_activation_storage_alignment));
    if (base == nullptr) {
      throw std::bad_alloc();
    }
    std::fill_n(
        reinterpret_cast<void **>(base + data.activation_layout.values_offset),
        data.values.size(), nullptr);
    std::fill_n(reinterpret_cast<void **>(
                    base + data.activation_layout.arguments_offset),
                data.node_storage.accesses.size(), nullptr);
    auto *generations = reinterpret_cast<std::optional<ResourceGeneration> *>(
        base + data.activation_layout.generations_offset);
    auto *previous_generations =
        reinterpret_cast<std::optional<ResourceGeneration> *>(
            base + data.activation_layout.previous_generations_offset);
    for (std::size_t resource = 0; resource < data.resources.size();
         ++resource) {
      std::construct_at(generations + resource, std::nullopt);
      std::construct_at(previous_generations + resource, std::nullopt);
    }
    std::fill_n(reinterpret_cast<std::uint8_t *>(
                    base + data.activation_layout.changed_values_offset),
                data.values.size(), std::uint8_t{0});
    const auto activation = m_activation_block_count++;
    m_activation_blocks[activation] = base;
    m_free_activations[m_free_activation_count++] = activation;
  }

  /// called by submitting thread
  /// called by worker thread
  /// per-node hot path
  void **activation_values(const ProgramData &data,
                           std::uint32_t activation) const {
    return reinterpret_cast<void **>(activation_base(data, activation) +
                                     data.activation_layout.values_offset);
  }

  /// called by submitting thread
  /// called by worker thread
  /// per-callback hot path
  void **activation_arguments(const ProgramData &data,
                              std::uint32_t activation) const {
    return reinterpret_cast<void **>(activation_base(data, activation) +
                                     data.activation_layout.arguments_offset);
  }

  /// called by submitting thread
  /// called by worker thread
  std::optional<ResourceGeneration> *
  activation_generations(const ProgramData &data,
                         std::uint32_t activation) const {
    return reinterpret_cast<std::optional<ResourceGeneration> *>(
        activation_base(data, activation) +
        data.activation_layout.generations_offset);
  }

  /// called by submitting thread
  /// called by worker thread
  std::optional<ResourceGeneration> *
  activation_previous_generations(const ProgramData &data,
                                  std::uint32_t activation) const {
    return reinterpret_cast<std::optional<ResourceGeneration> *>(
        activation_base(data, activation) +
        data.activation_layout.previous_generations_offset);
  }

  /// called by submitting thread
  /// called by worker thread
  /// per-node hot path
  std::uint8_t *activation_changed(const ProgramData &data,
                                   std::uint32_t activation) const {
    return reinterpret_cast<std::uint8_t *>(
        activation_base(data, activation) +
        data.activation_layout.changed_values_offset);
  }

  /// called by submitting thread
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
    for (std::uint32_t activation = 0; activation < m_activation_block_count;
         ++activation) {
      allocator_ref{}.deallocate(m_activation_blocks[activation],
                                 m_activation_block_size,
                                 m_activation_storage_alignment);
      m_activation_blocks[activation] = nullptr;
    }
    m_activation_block_count = 0;
    m_free_activation_count = 0;
    if (m_immediate_storage != nullptr) {
      allocator_ref{}.deallocate(m_immediate_storage, m_immediate_storage_size,
                                 m_immediate_storage_alignment);
      m_immediate_storage = nullptr;
    }
  }

  /// called by submitting thread
  /// called by worker thread
  std::optional<ResourceGeneration> try_acquire_locked(ResourceId resource) {
    assert(resource < m_resources.size());
    assert(m_resources[resource].slot_count != 0);
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
      ++m_physical_occupancy[m_slot_physical_ids[runtime.first_slot + slot]];
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

  /// called by submitting thread
  /// called by worker thread
  std::optional<ResourceGeneration> try_acquire_locked(ResourceId resource,
                                                       std::uint32_t slot) {
    assert(resource < m_resources.size());
    assert(slot < m_resources[resource].slot_count);
    auto &runtime = m_resources[resource];
    auto &state = m_slots[runtime.first_slot + slot];
    if (state.occupied || (runtime.preconstructed && !state.constructed) ||
        physical_range_occupied(resource, slot)) {
      return std::nullopt;
    }
    state.occupied = true;
    ++m_physical_occupancy[m_slot_physical_ids[runtime.first_slot + slot]];
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

  /// called by submitting thread
  /// called by worker thread
  bool physical_range_occupied(ResourceId resource, std::uint32_t slot) const {
    const auto logical_slot = m_resources[resource].first_slot + slot;
    assert(logical_slot < m_slot_physical_ids.size());
    const auto physical_slot = m_slot_physical_ids[logical_slot];
    assert(physical_slot < m_physical_occupancy.size());
    return m_physical_occupancy[physical_slot] != 0;
  }

  /// called by submitting thread
  /// called by worker thread
  RuntimeSlot &validate(const ResourceGeneration &generation) {
    assert(generation.resource < m_resources.size());
    const auto &resource = m_resources[generation.resource];
    assert(generation.slot < resource.slot_count);
    auto &slot = m_slots[resource.first_slot + generation.slot];
    assert(slot.occupied);
    assert(slot.generation == generation.generation);
    return slot;
  }

  /// called by submitting thread
  /// called by worker thread
  const RuntimeSlot &validate(const ResourceGeneration &generation) const {
    assert(generation.resource < m_resources.size());
    const auto &resource = m_resources[generation.resource];
    assert(generation.slot < resource.slot_count);
    const auto &slot = m_slots[resource.first_slot + generation.slot];
    assert(slot.occupied);
    assert(slot.generation == generation.generation);
    return slot;
  }

  /// called by submitting thread
  /// called by worker thread
  void *pointer(ResourceId resource, std::uint32_t slot) const {
    const auto &runtime = m_resources[resource];
    return static_cast<std::byte *>(m_storage) + runtime.offset +
           runtime.stride * slot;
  }

  void *m_storage = nullptr;
  std::size_t m_storage_size = 0;
  std::size_t m_storage_alignment = 1;
  std::array<void *, activation_pool_capacity> m_activation_blocks{};
  std::array<std::uint32_t, activation_pool_capacity> m_free_activations{};
  std::uint32_t m_activation_block_count = 0;
  std::uint32_t m_free_activation_count = 0;
  std::size_t m_activation_block_size = 0;
  std::size_t m_activation_storage_alignment = 1;
  void *m_immediate_storage = nullptr;
  std::size_t m_immediate_storage_size = 0;
  std::size_t m_immediate_storage_alignment = 1;
  std::size_t m_activation_generations_offset = 0;
  std::size_t m_activation_previous_generations_offset = 0;
  Vector<RuntimeResource, allocator_ref> m_resources;
  Vector<RuntimeSlot, allocator_ref> m_slots;
  Vector<Vector<ValueId, allocator_ref>, allocator_ref> m_values_by_resource;
  Vector<std::uint32_t, allocator_ref> m_projection_by_resource;
  Vector<std::uint32_t, allocator_ref> m_cutoff_by_value;
  Vector<std::uint32_t, allocator_ref> m_slot_physical_ids;
  Vector<std::uint32_t, allocator_ref> m_physical_occupancy;
  Vector<void *, allocator_ref> m_external_bindings;
  Vector<void *, allocator_ref> m_immediate_pointers;
  Vector<std::uint32_t, allocator_ref> m_immediate_counts;
  Vector<std::uint64_t, allocator_ref> m_semaphore_tokens;
  Vector<std::optional<ResourceGeneration>, allocator_ref> m_active_generations;
  Vector<std::optional<ResourceGeneration>, allocator_ref>
      m_feedback_generations;
  Vector<bool, allocator_ref> m_feedback_phi_is_external;
  Vector<Vector<std::uint32_t, allocator_ref>, allocator_ref>
      m_feedback_phis_by_value;
  Vector<bool, allocator_ref> m_activation_occupied;
  Vector<bool, allocator_ref> m_activation_running;
  Vector<std::uint64_t, allocator_ref> m_activation_sequences;
  Vector<RuntimeCutoff, allocator_ref> m_cutoffs;
  std::exception_ptr m_error;
  std::atomic<bool> m_failed = false;
  std::uint64_t m_next_activation = 0;
  mutable std::mutex m_mutex;
  std::condition_variable m_available;
};

} // namespace strobe::flow::detail
