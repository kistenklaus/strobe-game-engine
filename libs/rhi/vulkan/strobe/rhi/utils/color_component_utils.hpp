#pragma once

#include "strobe/rhi/types/color_component.hpp"

#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkColorComponentFlags
to_vk_color_component(ColorComponent components) noexcept {
  VkColorComponentFlags result = 0;

  if ((components & ColorComponent::r) != 0) {
    result |= VK_COLOR_COMPONENT_R_BIT;
  }

  if ((components & ColorComponent::g) != 0) {
    result |= VK_COLOR_COMPONENT_G_BIT;
  }

  if ((components & ColorComponent::b) != 0) {
    result |= VK_COLOR_COMPONENT_B_BIT;
  }

  if ((components & ColorComponent::a) != 0) {
    result |= VK_COLOR_COMPONENT_A_BIT;
  }

  return result;
}

static inline ColorComponent
from_vk_color_component(VkColorComponentFlags components) noexcept {
  ColorComponent result = ColorComponent::none;

  if ((components & VK_COLOR_COMPONENT_R_BIT) != 0) {
    result |= ColorComponent::r;
  }

  if ((components & VK_COLOR_COMPONENT_G_BIT) != 0) {
    result |= ColorComponent::g;
  }

  if ((components & VK_COLOR_COMPONENT_B_BIT) != 0) {
    result |= ColorComponent::b;
  }

  if ((components & VK_COLOR_COMPONENT_A_BIT) != 0) {
    result |= ColorComponent::a;
  }

  return result;
}

} // namespace strobe::rhi
