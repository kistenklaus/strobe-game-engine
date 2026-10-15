#pragma once

#include "strobe/flow/detail/compiler/slot_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

namespace strobe::flow::detail::compiler {

inline constexpr std::size_t no_storage_offset =
    std::numeric_limits<std::size_t>::max();

struct ResourceStorageLayout {
  std::size_t offset;
  std::size_t slot_stride;
  std::uint32_t slot_count;
  bool bounded_overlap;
};

struct StorageLayoutIR {
  StorageLayoutIR(SlotIR input,
                  Vector<ResourceStorageLayout, allocator_ref> resources,
                  std::size_t size, std::size_t alignment)
      : input(std::move(input)), resources(std::move(resources)), size(size),
        alignment(alignment) {}

  SlotIR input;
  Vector<ResourceStorageLayout, allocator_ref> resources;
  std::size_t size;
  std::size_t alignment;
};

} // namespace strobe::flow::detail::compiler
