#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/gpu/vulkan/allocator.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"
#include "strobe/gpu/vulkan/device_info/device_info.hpp"

#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace strobe::gpu::vulkan {

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
Vector<QueueLocation, vulkan::allocator_ref>
select_queues(const DeviceInfo<vulkan::allocator_ref> *device_info,
              const ContextCreateInfo *info,
              const vulkan::allocator_ref &alloc);

} // namespace strobe::gpu::vulkan
