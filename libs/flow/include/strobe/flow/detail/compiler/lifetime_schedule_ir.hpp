#pragma once

#include "strobe/flow/detail/compiler/storage_layout_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <utility>

namespace strobe::flow::detail::compiler {

struct ResourceLifetimeSchedule {
  IndexRange constructions;
  IndexRange release_after;
  bool repeating;
  bool preconstructed;
  bool retained_by_feedback;
  bool destroy_at_shutdown;
  void (*destroy)(void *) noexcept;
};

struct LifetimeScheduleIR {
  LifetimeScheduleIR(StorageLayoutIR input,
                     Vector<ResourceLifetimeSchedule, allocator_ref> resources,
                     Vector<NodeId, allocator_ref> constructions,
                     Vector<NodeId, allocator_ref> release_after)
      : input(std::move(input)), resources(std::move(resources)),
        constructions(std::move(constructions)),
        release_after(std::move(release_after)) {}

  StorageLayoutIR input;
  Vector<ResourceLifetimeSchedule, allocator_ref> resources;
  Vector<NodeId, allocator_ref> constructions;
  Vector<NodeId, allocator_ref> release_after;
};

} // namespace strobe::flow::detail::compiler
