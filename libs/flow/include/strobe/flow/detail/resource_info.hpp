#pragma once

#include "strobe/flow/type_id.hpp"

#include <strobe/core/containers/string.hpp>

#include <cstddef>
#include <optional>

namespace strobe::flow::detail {

struct ResourceInfo {
  TypeId type;
  String<allocator_ref> type_name;
  String<allocator_ref> label;
  std::size_t size;
  std::size_t alignment;
  void (*destroy)(void *) noexcept;
  std::optional<std::size_t> construction;
  std::optional<std::size_t> phi;
  std::optional<std::size_t> cutoff;
  std::optional<std::size_t> external;
  std::optional<std::size_t> projection;
};

} // namespace strobe::flow::detail
