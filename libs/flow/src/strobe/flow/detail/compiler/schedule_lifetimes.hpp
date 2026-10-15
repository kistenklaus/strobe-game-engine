#pragma once

#include "strobe/flow/detail/compiler/lifetime_schedule_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline LifetimeScheduleIR schedule_lifetimes(StorageLayoutIR input) {
  const auto &lifetimes = input.input.input.input;
  const auto &storage = lifetimes.input;
  Vector<ResourceLifetimeSchedule, allocator_ref> resources;
  Vector<NodeId, allocator_ref> constructions;
  Vector<NodeId, allocator_ref> release_after;
  resources.reserve(lifetimes.resources.size());

  for (ResourceId resource = 0; resource < lifetimes.resources.size();
       ++resource) {
    const auto &lifetime = lifetimes.resources[resource];
    if (lifetime.definitions.offset + lifetime.definitions.count >
            lifetimes.definitions.size() ||
        lifetime.last_uses.offset + lifetime.last_uses.count >
            lifetimes.last_uses.size()) {
      throw std::logic_error("resource lifetime range is out of bounds");
    }
    const auto construction_offset = constructions.size();
    for (std::uint32_t index = 0; index < lifetime.definitions.count; ++index) {
      constructions.push_back(
          lifetimes.definitions[lifetime.definitions.offset + index]);
    }
    const auto release_offset = release_after.size();
    for (std::uint32_t index = 0; index < lifetime.last_uses.count; ++index) {
      release_after.push_back(
          lifetimes.last_uses[lifetime.last_uses.offset + index]);
    }
    const bool owned = lifetime.storage == StorageClass::transient ||
                       lifetime.storage == StorageClass::persistent;
    resources.push_back(ResourceLifetimeSchedule{
        .constructions =
            {
                .offset = static_cast<std::uint32_t>(construction_offset),
                .count = static_cast<std::uint32_t>(lifetime.definitions.count),
            },
        .release_after =
            {
                .offset = static_cast<std::uint32_t>(release_offset),
                .count = static_cast<std::uint32_t>(lifetime.last_uses.count),
            },
        .repeating = lifetime.may_overlap_generations,
        .preconstructed = lifetime.preconstructed,
        .retained_by_feedback = lifetime.retained_by_feedback,
        .destroy_at_shutdown = owned && lifetime.destroy_at_shutdown,
        .destroy = owned ? storage.resources[resource].destroy : nullptr,
    });
  }

  return LifetimeScheduleIR(std::move(input), std::move(resources),
                            std::move(constructions), std::move(release_after));
}

} // namespace strobe::flow::detail::compiler
