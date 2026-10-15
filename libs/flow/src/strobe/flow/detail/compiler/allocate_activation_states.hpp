#pragma once

#include "strobe/flow/compile_options.hpp"
#include "strobe/flow/detail/compiler/activation_layout_ir.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>

namespace strobe::flow::detail::compiler {

inline ActivationLayoutIR
allocate_activation_states(LifetimeScheduleIR input,
                           const CompileOptions &options = {}) {
  if (options.max_slots == 0) {
    throw std::invalid_argument("Flow max_slots must be greater than zero");
  }
  const auto &layout = input.input;
  const auto &slots = layout.input;
  const auto &interference = slots.input;
  const auto &lifetimes = interference.input;
  const auto &storage = lifetimes.input;
  const auto &activation = storage.input;
  const auto &components = activation.input;
  const auto &values =
      components.input.classified.ordered.dependencies.lowered.resolved.values;
  bool drains_tokens = false;
  for (const auto &node : values.nodes) {
    drains_tokens |= node.kind == NodeKind::acquire;
  }
  bool replaces_persistent_value = false;
  for (const auto &lifetime : lifetimes.resources) {
    replaces_persistent_value |= lifetime.storage == StorageClass::persistent &&
                                 lifetime.memoized_reconstruction;
  }
  const auto capacity = drains_tokens && replaces_persistent_value
                            ? options.max_slots - 1
                        : drains_tokens ? options.max_slots
                                        : 1U;
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
