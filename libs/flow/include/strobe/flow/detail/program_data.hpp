#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/allocator.hpp"
#include "strobe/flow/compile_context.hpp"
#include "strobe/flow/detail/operation_kind.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/op_source.hpp"
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
  IndexRange predecessors;
  IndexRange successors;
  IndexRange temporal_predecessors;
  IndexRange temporal_successors;
  bool runs_once = false;
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
  ValueId input;
  ValueId output;
};

struct CompiledExternal {
  ResourceId resource;
  String<allocator_ref> name;
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

using RuntimeCallback =
    std::move_only_function<void(Lifetime, span<void *const>)>;

struct CompiledCallback {
  CompiledCallback() = default;

  template <typename F>
    requires std::constructible_from<RuntimeCallback, F>
  CompiledCallback(F &&callback) : runtime(std::forward<F>(callback)) {}

  OpMetadata metadata;
  std::optional<ExternalOp> external;
  Vector<TypeId, allocator_ref> argument_types;
  Vector<AccessMode, allocator_ref> argument_accesses;
  RuntimeCallback runtime;
};

struct ProgramData {
  CompileContext plugin_data;
  ActivationLayout activation_layout;
  Vector<CompiledNode, allocator_ref> nodes;
  Vector<ValueId, allocator_ref> inputs;
  Vector<ValueId, allocator_ref> outputs;
  Vector<NodeId, allocator_ref> predecessors;
  Vector<NodeId, allocator_ref> successors;
  Vector<NodeId, allocator_ref> temporal_predecessors;
  Vector<NodeId, allocator_ref> temporal_successors;
  Vector<CompiledValue, allocator_ref> values;
  Vector<CompiledAccess, allocator_ref> accesses;
  Vector<CompiledExternal, allocator_ref> externals;
  Vector<CompiledPhi, allocator_ref> phis;
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
  Vector<NodeId, allocator_ref> roots;
  Vector<NodeId, allocator_ref> yields;
  Vector<TypeId, allocator_ref> resource_types;
  Vector<ResourceSlotPlan, allocator_ref> resource_slots;
  Vector<ResourceLifetimePlan, allocator_ref> resource_lifetimes;
  Vector<NodeId, allocator_ref> constructions;
  Vector<NodeId, allocator_ref> release_after;

  std::size_t storage_size = 0;
  std::size_t storage_alignment = 1;

  std::size_t source_operation_count = 0;
  std::size_t source_resource_count = 0;
  std::size_t source_external_count = 0;
};

} // namespace strobe::flow::detail
