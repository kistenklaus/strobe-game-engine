#pragma once

#include "strobe/flow/detail/compiler/interference_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <cstdint>
#include <utility>

namespace strobe::flow::detail::compiler {

struct ResourceSlotInfo {
  std::uint32_t slot_count;
  bool bounded_overlap;
};

struct SlotIR {
  SlotIR(InterferenceIR input,
         Vector<ResourceSlotInfo, allocator_ref> resources)
      : input(std::move(input)), resources(std::move(resources)) {}

  InterferenceIR input;
  Vector<ResourceSlotInfo, allocator_ref> resources;
};

} // namespace strobe::flow::detail::compiler
