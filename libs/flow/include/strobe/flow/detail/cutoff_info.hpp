#pragma once

#include "strobe/flow/resource_id.hpp"

#include <cstddef>
#include <functional>

namespace strobe::flow::detail {

struct CutoffInfo {
  ResourceId input;
  ResourceId output;
  std::size_t operation;
  std::move_only_function<bool(const void *, const void *) const> equal;
  std::move_only_function<void(void *, const void *) const> copy_construct;
  void (*destroy)(void *) noexcept;
  std::size_t size;
  std::size_t alignment;
};

} // namespace strobe::flow::detail
