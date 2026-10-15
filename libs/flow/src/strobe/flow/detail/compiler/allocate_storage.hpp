#pragma once

#include "strobe/flow/detail/compiler/storage_layout_ir.hpp"

#include <strobe/core/containers/vector.hpp>
#include <strobe/core/graph.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline StorageLayoutIR allocate_storage(SlotIR input) {
  const auto &interference = input.input;
  Vector<ResourceStorageLayout, allocator_ref> resources;
  resources.resize(input.resources.size(), ResourceStorageLayout{
                                               .offset = no_storage_offset,
                                               .slot_stride = 0,
                                               .slot_count = 0,
                                               .bounded_overlap = false,
                                           });

  Vector<graph::NodeId, allocator_ref> order;
  order.reserve(interference.graph.node_count());
  for (std::uint32_t node = 0; node < interference.graph.node_count(); ++node) {
    order.push_back(graph::NodeId{node});
  }

  const auto checked_align = [](std::size_t value, std::size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
      throw std::logic_error("resource alignment is not a power of two");
    }
    if (value > std::numeric_limits<std::size_t>::max() - (alignment - 1)) {
      throw std::length_error("Flow storage layout size overflow");
    }
    return (value + alignment - 1) & ~(alignment - 1);
  };
  const auto allocation_size = [&](graph::NodeId node) {
    const auto &payload = interference.graph.get(node);
    const auto slots = input.resources[payload.resource].slot_count;
    const auto stride = checked_align(payload.size, payload.alignment);
    if (slots != 0 &&
        stride > std::numeric_limits<std::size_t>::max() / slots) {
      throw std::length_error("Flow storage layout size overflow");
    }
    return stride * slots;
  };
  std::sort(order.begin(), order.end(), [&](auto left, auto right) {
    const auto left_size = allocation_size(left);
    const auto right_size = allocation_size(right);
    if (left_size != right_size) {
      return left_size > right_size;
    }
    return interference.graph.get(left).alignment >
           interference.graph.get(right).alignment;
  });

  const auto interferes = [&](graph::NodeId left, graph::NodeId right) {
    for (const auto edge : interference.graph.outgoing(left)) {
      if (interference.graph.destination(edge) == right) {
        return true;
      }
    }
    for (const auto edge : interference.graph.incoming(left)) {
      if (interference.graph.source(edge) == right) {
        return true;
      }
    }
    return false;
  };

  std::size_t total_size = 0;
  std::size_t total_alignment = 1;
  Vector<graph::NodeId, allocator_ref> placed;
  for (const auto node : order) {
    const auto &payload = interference.graph.get(node);
    const auto &slot = input.resources[payload.resource];
    const auto stride = checked_align(payload.size, payload.alignment);
    const auto bytes = allocation_size(node);
    std::size_t offset = 0;
    while (true) {
      offset = checked_align(offset, payload.alignment);
      if (offset > std::numeric_limits<std::size_t>::max() - bytes) {
        throw std::length_error("Flow storage layout size overflow");
      }
      const auto end = offset + bytes;
      bool moved = false;
      for (const auto other : placed) {
        if (!interferes(node, other)) {
          continue;
        }
        const auto other_resource = interference.graph.get(other).resource;
        const auto &other_layout = resources[other_resource];
        const auto other_bytes =
            other_layout.slot_stride * other_layout.slot_count;
        if (offset < other_layout.offset + other_bytes &&
            other_layout.offset < end) {
          offset = other_layout.offset + other_bytes;
          moved = true;
          break;
        }
      }
      if (!moved) {
        break;
      }
    }
    resources[payload.resource] = ResourceStorageLayout{
        .offset = offset,
        .slot_stride = stride,
        .slot_count = slot.slot_count,
        .bounded_overlap = slot.bounded_overlap,
    };
    total_size = std::max(total_size, offset + bytes);
    total_alignment = std::max(total_alignment, payload.alignment);
    placed.push_back(node);
  }

  if (total_size != 0) {
    total_size = checked_align(total_size, total_alignment);
  }
  return StorageLayoutIR(std::move(input), std::move(resources), total_size,
                         total_alignment);
}

} // namespace strobe::flow::detail::compiler
