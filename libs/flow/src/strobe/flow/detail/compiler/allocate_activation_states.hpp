#pragma once

#include "strobe/flow/detail/compiler/activation_layout_ir.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>

namespace strobe::flow::detail::compiler {

inline ActivationLayoutIR allocate_activation_states(LifetimeScheduleIR input) {
  const auto &layout = input.input;
  const auto &slots = layout.input;
  const auto &interference = slots.input;
  const auto &lifetimes = interference.input;
  const auto &storage = lifetimes.input;
  const auto &activation = storage.input;
  const auto &components = activation.input;
  const auto &values =
      components.input.classified.ordered.dependencies.lowered.resolved.values;
  std::uint32_t capacity = 0;
  for (std::size_t phi = 0; phi < values.phis.size(); ++phi) {
    const auto &region = values.phis[phi];
    if (region.condition != null_value_id) {
      continue;
    }
    bool first_in_region = true;
    for (std::size_t prior = 0; prior < phi; ++prior) {
      first_in_region &=
          values.phis[prior].declaration_position !=
              region.declaration_position ||
          values.phis[prior].feedback_position != region.feedback_position;
    }
    if (!first_in_region) {
      continue;
    }
    if (capacity > std::numeric_limits<std::uint32_t>::max() -
                       region.max_in_flight) {
      throw std::length_error("Flow activation capacity overflow");
    }
    capacity += region.max_in_flight;
  }
  capacity = std::max(capacity, 1U);
  if (capacity == 0) {
    throw std::invalid_argument("Flow activation capacity must be nonzero");
  }

  std::size_t offset = 0;
  std::size_t alignment = 1;
  const auto append = [&](std::size_t count, std::size_t element_size,
                          std::size_t element_alignment) {
    alignment = std::max(alignment, element_alignment);
    if (offset >
        std::numeric_limits<std::size_t>::max() - (element_alignment - 1)) {
      throw std::length_error("Flow activation layout overflow");
    }
    offset = (offset + element_alignment - 1) & ~(element_alignment - 1);
    const auto result = offset;
    if (count != 0 &&
        element_size > std::numeric_limits<std::size_t>::max() / count) {
      throw std::length_error("Flow activation layout overflow");
    }
    const auto bytes = count * element_size;
    if (offset > std::numeric_limits<std::size_t>::max() - bytes) {
      throw std::length_error("Flow activation layout overflow");
    }
    offset += bytes;
    return result;
  };

  const auto values_offset =
      append(values.values.size(), sizeof(void *), alignof(void *));
  const auto arguments_offset =
      append(values.accesses.size(), sizeof(void *), alignof(void *));
  const auto generations_offset =
      append(values.resources.size(), sizeof(std::optional<ResourceGeneration>),
             alignof(std::optional<ResourceGeneration>));
  const auto previous_generations_offset =
      append(values.resources.size(), sizeof(std::optional<ResourceGeneration>),
             alignof(std::optional<ResourceGeneration>));
  const auto changed_values_offset =
      append(values.values.size(), sizeof(std::uint8_t), alignof(std::uint8_t));
  offset = std::max(offset, std::size_t{1});
  offset = (offset + alignment - 1) & ~(alignment - 1);
  if (offset != 0 &&
      capacity > std::numeric_limits<std::size_t>::max() / offset) {
    throw std::length_error("Flow activation layout overflow");
  }
  return ActivationLayoutIR{
      .input = std::move(input),
      .capacity = capacity,
      .stride = offset,
      .values_offset = values_offset,
      .arguments_offset = arguments_offset,
      .generations_offset = generations_offset,
      .previous_generations_offset = previous_generations_offset,
      .changed_values_offset = changed_values_offset,
      .size = offset * capacity,
      .alignment = alignment,
  };
}

} // namespace strobe::flow::detail::compiler
