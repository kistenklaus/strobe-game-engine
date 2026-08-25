#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/vulkan/context/create_info.hpp"
#include "strobe/rhi/vulkan/device_info/device_info.hpp"

#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace strobe::rhi::vulkan {

struct QueueLocation {
  static constexpr uint32_t invalid = std::numeric_limits<uint32_t>::max();

  uint32_t family = invalid;
  uint32_t index = invalid;

  [[nodiscard]]
  constexpr operator bool() const noexcept {
    return family != invalid && index != invalid;
  }
};

[[nodiscard]]
Vector<QueueLocation, strobe::rhi::allocator_ref>
select_queues(const DeviceInfo *device_info, const ContextCreateInfo *info,
              const strobe::rhi::allocator_ref &alloc);

} // namespace strobe::rhi::vulkan
