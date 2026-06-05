#pragma once

#include "strobe/ecs/cmd/cmd_domain.hpp"
#include <concepts>
namespace strobe::ecs {

template <typename T>
concept command_buffer = requires(const T &c, T &m) {
  typename T::scope;
  { c.peek() } noexcept -> std::convertible_to<cmd_index>;
  { m.step() } noexcept;
  { c.domain() } noexcept -> std::same_as<const cmd_domain *>;
};

} // namespace strobe::ecs
