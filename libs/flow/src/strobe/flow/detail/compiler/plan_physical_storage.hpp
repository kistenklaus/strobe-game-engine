#pragma once

#include "strobe/flow/detail/program_data.hpp"

#include <strobe/core/containers/vector.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace strobe::flow::detail::compiler {

inline void plan_physical_storage(ProgramData &data) {
  const auto resource_count = data.resources.size();
  auto &plan = data.physical_storage;
  plan.resource_slot_offsets.clear();
  plan.resource_slot_offsets.resize(resource_count + 1, 0);
  for (ResourceId resource = 0; resource < resource_count; ++resource) {
    plan.resource_slot_offsets[resource + 1] =
        plan.resource_slot_offsets[resource] +
        data.resources[resource].slots.slot_count;
  }

  const auto logical_slot_count = plan.resource_slot_offsets.back();
  Vector<std::uint32_t, allocator_ref> parents(logical_slot_count, 0,
                                               allocator_ref{});
  for (std::uint32_t slot = 0; slot < logical_slot_count; ++slot) {
    parents[slot] = slot;
  }
  const auto find = [&](std::uint32_t slot) {
    auto root = slot;
    while (parents[root] != root) {
      root = parents[root];
    }
    while (parents[slot] != slot) {
      const auto parent = parents[slot];
      parents[slot] = root;
      slot = parent;
    }
    return root;
  };

  for (ResourceId left_resource = 0; left_resource < resource_count;
       ++left_resource) {
    const auto &left = data.resources[left_resource].slots;
    if (left.offset == null_storage_offset) {
      continue;
    }
    for (std::uint32_t left_slot = 0; left_slot < left.slot_count;
         ++left_slot) {
      const auto left_begin = left.offset + left.slot_stride * left_slot;
      const auto left_end = left_begin + left.slot_stride;
      const auto left_id =
          plan.resource_slot_offsets[left_resource] + left_slot;
      for (ResourceId right_resource = left_resource;
           right_resource < resource_count; ++right_resource) {
        const auto &right = data.resources[right_resource].slots;
        if (right.offset == null_storage_offset) {
          continue;
        }
        const auto first_right_slot =
            right_resource == left_resource ? left_slot + 1 : 0;
        for (std::uint32_t right_slot = first_right_slot;
             right_slot < right.slot_count; ++right_slot) {
          const auto right_begin =
              right.offset + right.slot_stride * right_slot;
          const auto right_end = right_begin + right.slot_stride;
          if (left_begin >= right_end || right_begin >= left_end) {
            continue;
          }
          const auto right_id =
              plan.resource_slot_offsets[right_resource] + right_slot;
          const auto left_root = find(left_id);
          const auto right_root = find(right_id);
          if (left_root != right_root) {
            parents[right_root] = left_root;
          }
        }
      }
    }
  }

  plan.slot_physical_ids.clear();
  plan.slot_physical_ids.resize(logical_slot_count, null_node_id);
  std::uint32_t physical_slot_count = 0;
  for (std::uint32_t slot = 0; slot < logical_slot_count; ++slot) {
    const auto root = find(slot);
    if (plan.slot_physical_ids[root] == null_node_id) {
      plan.slot_physical_ids[root] = physical_slot_count++;
    }
    plan.slot_physical_ids[slot] = plan.slot_physical_ids[root];
  }

  Vector<std::uint32_t, allocator_ref> frontier_capacities(physical_slot_count,
                                                           1, allocator_ref{});
  if (data.resources.size() != resource_count) {
    throw std::logic_error(
        "physical storage resources and lifetimes do not match");
  }
  for (ResourceId resource = 0; resource < resource_count; ++resource) {
    const auto capacity = static_cast<std::uint32_t>(std::max<std::size_t>(
        1, data.resources[resource].lifetime.release_after.count));
    for (auto slot = plan.resource_slot_offsets[resource];
         slot < plan.resource_slot_offsets[resource + 1]; ++slot) {
      frontier_capacities[plan.slot_physical_ids[slot]] =
          std::max(frontier_capacities[plan.slot_physical_ids[slot]], capacity);
    }
  }

  plan.frontier_offsets.clear();
  plan.frontier_offsets.resize(physical_slot_count + 1, 0);
  for (std::uint32_t slot = 0; slot < physical_slot_count; ++slot) {
    plan.frontier_offsets[slot + 1] =
        plan.frontier_offsets[slot] + frontier_capacities[slot];
  }
}

} // namespace strobe::flow::detail::compiler
