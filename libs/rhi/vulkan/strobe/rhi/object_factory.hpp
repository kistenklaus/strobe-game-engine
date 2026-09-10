#pragma once

#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi::detail {

template <typename T> T make_object(void *handle) noexcept {
  return Object<T>::from_handle(handle);
}

} // namespace strobe::rhi::detail
