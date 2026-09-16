#pragma once

#include "strobe/rhi/types/border_color.hpp"
#include <exception>
#include <fmt/printf.h>
#include <utility>
#include <vulkan/vulkan_core.h>
namespace strobe::rhi {

static inline VkBorderColor to_vk_border_color(BorderColor color) noexcept {
  switch (color) {
  case BorderColor::float_transparent:
    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  case BorderColor::int_transparent:
    return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
  case BorderColor::float_opaque_black:
    return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
  case BorderColor::int_opaque_black:
    return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  case BorderColor::float_opaque_white:
    return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  case BorderColor::int_opaque_white:
    return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  }
  std::unreachable();
}

static inline BorderColor from_vk_border_color(VkBorderColor color) noexcept {
  switch (color) {
  case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
    return BorderColor::float_transparent;
  case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:
    return BorderColor::int_transparent;
  case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
    return BorderColor::float_opaque_black;
  case VK_BORDER_COLOR_INT_OPAQUE_BLACK:
    return BorderColor::int_opaque_black;
  case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
    return BorderColor::float_opaque_white;
  case VK_BORDER_COLOR_INT_OPAQUE_WHITE:
    return BorderColor::int_opaque_white;
  case VK_BORDER_COLOR_FLOAT_CUSTOM_EXT:
  case VK_BORDER_COLOR_INT_CUSTOM_EXT:
  case VK_BORDER_COLOR_MAX_ENUM:
    fmt::println("Invalid VkBorderColor");
    std::terminate();
  }
  std::unreachable();
}

} // namespace strobe::rhi
