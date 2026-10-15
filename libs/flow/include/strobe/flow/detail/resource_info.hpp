#pragma once

#include "strobe/flow/type_id.hpp"

#include <cstddef>
#include <optional>

namespace strobe::flow::detail {

struct ResourceInfo {
  TypeId type;
  std::size_t size;
  std::size_t alignment;
  void (*destroy)(void *) noexcept;
  std::optional<std::size_t> construction;
  std::optional<std::size_t> phi;
  std::optional<std::size_t> cutoff;
  std::optional<std::size_t> external;
};

} // namespace strobe::flow::detail
