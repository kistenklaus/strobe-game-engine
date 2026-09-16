#pragma once

#include "strobe/rhi/types/filter.hpp"
#include <exception>
#include <fmt/printf.h>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkFilter to_vk_filter(Filter filter) noexcept {
  switch (filter) {
  case Filter::nearest:
    return VK_FILTER_NEAREST;
  case Filter::linear:
    return VK_FILTER_LINEAR;
  }
  std::unreachable();
}

static inline Filter from_vk_filter(VkFilter filter) noexcept {
  switch (filter) {
  case VK_FILTER_NEAREST:
    return Filter::nearest;
  case VK_FILTER_LINEAR:
    return Filter::linear;
  case VK_FILTER_CUBIC_EXT:
  case VK_FILTER_MAX_ENUM:
    fmt::println("INVALID VkFilter");
    std::terminate();
  }
  std::unreachable();
}

} // namespace strobe::rhi
