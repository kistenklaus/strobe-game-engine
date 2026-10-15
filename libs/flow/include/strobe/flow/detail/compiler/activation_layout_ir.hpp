#pragma once

#include "strobe/flow/detail/compiler/lifetime_schedule_ir.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace strobe::flow::detail::compiler {

struct ActivationLayoutIR {
  LifetimeScheduleIR input;
  std::uint32_t capacity;
  std::size_t stride;
  std::size_t values_offset;
  std::size_t arguments_offset;
  std::size_t generations_offset;
  std::size_t previous_generations_offset;
  std::size_t changed_values_offset;
  std::size_t size;
  std::size_t alignment;
};

} // namespace strobe::flow::detail::compiler
