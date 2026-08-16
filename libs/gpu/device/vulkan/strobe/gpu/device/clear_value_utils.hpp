#pragma once

#include "strobe/gpu/device/clear_value.hpp"

#include <bit>
#include <type_traits>
#include <vulkan/vulkan.h>

namespace strobe::gpu {

static_assert(std::is_trivially_copyable_v<ClearValue>);
static_assert(std::is_trivially_copyable_v<VkClearValue>);
static_assert(sizeof(ClearValue) == sizeof(VkClearValue));

[[nodiscard]]
static inline VkClearValue
to_vk_clear_value(const ClearValue &value) noexcept {
  return std::bit_cast<VkClearValue>(value);
}

[[nodiscard]]
static inline ClearValue
from_vk_clear_value(const VkClearValue &value) noexcept {
  return std::bit_cast<ClearValue>(value);
}

} // namespace strobe::gpu
