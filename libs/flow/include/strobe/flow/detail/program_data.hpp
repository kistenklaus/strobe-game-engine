#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/access_purpose.hpp"
#include "strobe/flow/allocator.hpp"
#include "strobe/flow/compile_context.hpp"
#include "strobe/flow/detail/operation_kind.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/projection.hpp"
#include "strobe/flow/resource_id.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/string.hpp>
#include <strobe/core/containers/vector.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <utility>

namespace strobe::flow::detail {

using NodeId = std::uint32_t;
using ValueId = std::uint32_t;
using CallbackId = std::uint32_t;
using ControlId = std::uint32_t;

inline constexpr NodeId null_node_id = std::numeric_limits<NodeId>::max();
inline constexpr ValueId null_value_id = std::numeric_limits<ValueId>::max();
inline constexpr CallbackId null_callback_id =
    std::numeric_limits<CallbackId>::max();
inline constexpr ControlId null_control_id =
    std::numeric_limits<ControlId>::max();
inline constexpr std::size_t null_storage_offset =
    std::numeric_limits<std::size_t>::max();

struct IndexRange {
  std::uint32_t offset = 0;
  std::uint32_t count = 0;
};

struct ResourceSlotPlan {
  std::size_t offset = null_storage_offset;
  std::size_t slot_stride = 0;
  std::uint32_t slot_count = 0;
  bool bounded_overlap = false;
};

struct ResourceLifetimePlan {
  Lifetime interval;
  IndexRange constructions;
  IndexRange release_after;
  bool repeating = false;
  bool preconstructed = false;
  bool retained_by_feedback = false;
  bool destroy_at_shutdown = false;
  void (*destroy)(void *) noexcept = nullptr;
};

struct CompiledResource {
  TypeId type{};
  String<allocator_ref> type_name{allocator_ref{}};
  String<allocator_ref> label{allocator_ref{}};
  ResourceSlotPlan slots{};
  ResourceLifetimePlan lifetime{};
};

struct ActivationLayout {
  std::uint32_t capacity = 1;
  std::size_t stride = 0;
  std::size_t values_offset = 0;
  std::size_t arguments_offset = 0;
  std::size_t generations_offset = 0;
  std::size_t previous_generations_offset = 0;
  std::size_t changed_values_offset = 0;
  std::size_t size = 0;
  std::size_t alignment = 1;
};

struct CompiledNode {
  NodeId source = null_node_id;
  OperationKind kind = OperationKind::callback;
  CallbackId callback = null_callback_id;
  IndexRange inputs;
  IndexRange outputs;
  IndexRange accesses;
  std::uint32_t argument_count = 0;
  IndexRange predecessors;
  IndexRange successors;
  IndexRange temporal_predecessors;
  IndexRange temporal_successors;
  bool initialization = false;
  bool memoized = false;
};

struct CompiledValue {
  ResourceId resource;
  std::uint32_t version;
  bool external;
};

struct CompiledAccess {
  ResourceId resource;
  AccessMode mode;
  AccessPurpose purpose = AccessPurpose::argument;
  ValueId input;
  ValueId output;
};

struct CompiledNodeStorage {
  Vector<ValueId, allocator_ref> inputs;
  Vector<ValueId, allocator_ref> outputs;
  Vector<CompiledAccess, allocator_ref> accesses;
  Vector<NodeId, allocator_ref> predecessors;
  Vector<NodeId, allocator_ref> successors;
  Vector<NodeId, allocator_ref> temporal_predecessors;
  Vector<NodeId, allocator_ref> temporal_successors;
};

struct CompiledExternal {
  ResourceId resource;
  String<allocator_ref> name;
};

struct CompiledProjection {
  ResourceId resource;
  ResourceId owner;
  ProjectionKind kind;
  String<allocator_ref> name;
  std::move_only_function<void *(void *) const> resolve;
  std::optional<ExternalOp> external;
};

struct CompiledPhi {
  NodeId node;
  ValueId initial;
  ValueId feedback;
  ValueId output;
};

struct CompiledAcquire {
  NodeId node;
  std::uint32_t semaphore;
};

struct CompiledTokenRequirement {
  std::uint32_t semaphore;
  std::uint32_t count;
};

struct CompiledFeedbackRegion {
  std::uint32_t declaration_position;
  std::uint32_t feedback_position;
  IndexRange phis;
  IndexRange nodes;
  IndexRange acquires;
  IndexRange token_requirements;
  std::uint32_t max_in_flight = 2;
};

enum class ControlKind : std::uint8_t {
  loop,
  branch,
};

enum class ControlStepKind : std::uint8_t {
  node,
  control,
};

struct CompiledControlStep {
  ControlStepKind kind;
  std::uint32_t index;
};

struct CompiledControl {
  ControlKind kind = ControlKind::loop;
  ControlId parent = null_control_id;
  ValueId initial_condition = null_value_id;
  ValueId repeat_condition = null_value_id;
  NodeId initial_condition_node = null_node_id;
  IndexRange primary_steps;
  IndexRange secondary_steps;
  IndexRange nodes;
  IndexRange resources;
  IndexRange primary_written_resources;
  IndexRange secondary_written_resources;
};

struct CompiledCutoff {
  NodeId node;
  ValueId input;
  ValueId output;
  std::move_only_function<bool(const void *, const void *) const> equal;
  std::move_only_function<void(void *, const void *) const> copy_construct;
  void (*destroy)(void *) noexcept;
  std::size_t size;
  std::size_t alignment;
};

struct ResourceGeneration {
  ResourceId resource = null_resource_id;
  std::uint32_t slot = 0;
  std::uint64_t generation = 0;
};

struct CompiledImmediate {
  TypeId type;
  std::size_t offset;
  std::size_t size;
  std::size_t alignment;
  std::size_t element_size;
  std::uint32_t element_count;
};

using RuntimeCallback = std::move_only_function<void(
    Lifetime, span<void *const>, span<const std::uint32_t>,
    span<const ArgumentGroup>, span<const std::uint32_t>, span<void *const>)>;

struct CompiledCallback {
  CompiledCallback() = default;

  template <typename F>
    requires std::constructible_from<RuntimeCallback, F>
  CompiledCallback(F &&callback) : runtime(std::forward<F>(callback)) {}

  OpMetadata metadata;
  std::optional<ExternalOp> external;
  Vector<TypeId, allocator_ref> argument_types;
  Vector<AccessMode, allocator_ref> argument_accesses;
  IndexRange argument_groups;
  IndexRange argument_elements;
  IndexRange immediates;
  RuntimeCallback runtime;
};

struct PhysicalStoragePlan {
  Vector<std::uint32_t, allocator_ref> resource_slot_offsets;
  Vector<std::uint32_t, allocator_ref> slot_physical_ids;
  Vector<std::uint32_t, allocator_ref> frontier_offsets;
};

struct ProgramData {
  CompileContext plugin_data;
  ActivationLayout activation_layout;
  Vector<CompiledNode, allocator_ref> nodes;
  CompiledNodeStorage node_storage;
  Vector<CompiledValue, allocator_ref> values;
  Vector<CompiledExternal, allocator_ref> externals;
  Vector<CompiledProjection, allocator_ref> projections;
  Vector<CompiledPhi, allocator_ref> phis;
  Vector<CompiledFeedbackRegion, allocator_ref> feedback_regions;
  Vector<std::uint32_t, allocator_ref> feedback_region_phis;
  Vector<NodeId, allocator_ref> feedback_region_nodes;
  Vector<std::uint32_t, allocator_ref> feedback_region_acquires;
  Vector<CompiledTokenRequirement, allocator_ref> feedback_token_requirements;
  Vector<CompiledAcquire, allocator_ref> acquires;
  Vector<CompiledControl, allocator_ref> controls;
  Vector<CompiledControlStep, allocator_ref> control_steps;
  Vector<NodeId, allocator_ref> control_nodes;
  Vector<ResourceId, allocator_ref> control_resources;
  Vector<ResourceId, allocator_ref> control_written_resources;
  Vector<ControlId, allocator_ref> control_of_node;
  Vector<String<allocator_ref>, allocator_ref> semaphore_names;
  Vector<CompiledCutoff, allocator_ref> cutoffs;
  Vector<CompiledCallback, allocator_ref> callbacks;
  Vector<CompiledImmediate, allocator_ref> immediates;
  Vector<ArgumentGroup, allocator_ref> argument_groups;
  Vector<std::uint32_t, allocator_ref> argument_elements;
  Vector<std::byte, allocator_ref> immediate_data;
  std::size_t immediate_alignment = 1;
  Vector<NodeId, allocator_ref> roots;
  Vector<NodeId, allocator_ref> yields;
  Vector<CompiledResource, allocator_ref> resources;
  PhysicalStoragePlan physical_storage;
  Vector<NodeId, allocator_ref> constructions;
  Vector<NodeId, allocator_ref> release_after;

  std::size_t storage_size = 0;
  std::size_t storage_alignment = 1;

  std::size_t source_operation_count = 0;
  std::size_t source_resource_count = 0;
  std::size_t source_external_count = 0;

  span<CompiledAccess> accesses_of(CompiledNode &node) noexcept;
  span<const CompiledAccess>
  accesses_of(const CompiledNode &node) const noexcept;
  span<CompiledAccess> accesses_of(NodeId node) noexcept;
  span<const CompiledAccess> accesses_of(NodeId node) const noexcept;

  span<ValueId> inputs_of(CompiledNode &node) noexcept;
  span<const ValueId> inputs_of(const CompiledNode &node) const noexcept;
  span<ValueId> outputs_of(CompiledNode &node) noexcept;
  span<const ValueId> outputs_of(const CompiledNode &node) const noexcept;

  span<NodeId> predecessors_of(CompiledNode &node) noexcept;
  span<const NodeId> predecessors_of(const CompiledNode &node) const noexcept;
  span<NodeId> successors_of(CompiledNode &node) noexcept;
  span<const NodeId> successors_of(const CompiledNode &node) const noexcept;
  span<NodeId> temporal_predecessors_of(CompiledNode &node) noexcept;
  span<const NodeId>
  temporal_predecessors_of(const CompiledNode &node) const noexcept;
  span<NodeId> temporal_successors_of(CompiledNode &node) noexcept;
  span<const NodeId>
  temporal_successors_of(const CompiledNode &node) const noexcept;

  span<std::uint32_t> phis_of(CompiledFeedbackRegion &region) noexcept;
  span<const std::uint32_t>
  phis_of(const CompiledFeedbackRegion &region) const noexcept;
  span<NodeId> nodes_of(CompiledFeedbackRegion &region) noexcept;
  span<const NodeId>
  nodes_of(const CompiledFeedbackRegion &region) const noexcept;
  span<std::uint32_t> acquires_of(CompiledFeedbackRegion &region) noexcept;
  span<const std::uint32_t>
  acquires_of(const CompiledFeedbackRegion &region) const noexcept;
  span<CompiledTokenRequirement>
  token_requirements_of(CompiledFeedbackRegion &region) noexcept;
  span<const CompiledTokenRequirement>
  token_requirements_of(const CompiledFeedbackRegion &region) const noexcept;

  span<CompiledControlStep> primary_steps_of(CompiledControl &control) noexcept;
  span<const CompiledControlStep>
  primary_steps_of(const CompiledControl &control) const noexcept;
  span<CompiledControlStep>
  secondary_steps_of(CompiledControl &control) noexcept;
  span<const CompiledControlStep>
  secondary_steps_of(const CompiledControl &control) const noexcept;
  span<NodeId> nodes_of(CompiledControl &control) noexcept;
  span<const NodeId> nodes_of(const CompiledControl &control) const noexcept;
  span<ResourceId> resources_of(CompiledControl &control) noexcept;
  span<const ResourceId>
  resources_of(const CompiledControl &control) const noexcept;
  span<ResourceId>
  primary_written_resources_of(CompiledControl &control) noexcept;
  span<const ResourceId>
  primary_written_resources_of(const CompiledControl &control) const noexcept;
  span<ResourceId>
  secondary_written_resources_of(CompiledControl &control) noexcept;
  span<const ResourceId>
  secondary_written_resources_of(const CompiledControl &control) const noexcept;

  span<ArgumentGroup> argument_groups_of(CompiledCallback &callback) noexcept;
  span<const ArgumentGroup>
  argument_groups_of(const CompiledCallback &callback) const noexcept;
  span<std::uint32_t> argument_elements_of(CompiledCallback &callback) noexcept;
  span<const std::uint32_t>
  argument_elements_of(const CompiledCallback &callback) const noexcept;
  span<CompiledImmediate> immediates_of(CompiledCallback &callback) noexcept;
  span<const CompiledImmediate>
  immediates_of(const CompiledCallback &callback) const noexcept;

  span<NodeId> constructions_of(CompiledResource &resource) noexcept;
  span<const NodeId>
  constructions_of(const CompiledResource &resource) const noexcept;
  span<NodeId> release_after_of(CompiledResource &resource) noexcept;
  span<const NodeId>
  release_after_of(const CompiledResource &resource) const noexcept;
};

String<allocator_ref> to_pretty(const ProgramData &data);

} // namespace strobe::flow::detail
