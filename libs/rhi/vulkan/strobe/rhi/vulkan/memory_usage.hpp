#pragma once

#include "strobe/rhi/vulkan/memory_requirements.hpp"
#include <vk_mem_alloc.h>

namespace strobe::rhi::vulkan {

enum class MemoryUsage : uint32_t {
  automatic,
  device, // require DEVICE_LOCAL
  mapped, // HOST_VISIBLE, HOST_COHERENT persistent mapping, CPU reads/random
          // access
  mapped_write_sequential, // HOST_VISIBLE, HOST_COHERENT persistent mapping,
                           // sequential CPU writes
  mapped_incoherent, // not recommended, more here for legacy
};

namespace details {

[[nodiscard]]
const VmaAllocationCreateInfo *
get_auto_allocation_create_info(MemoryUsage usage) noexcept;

[[nodiscard]]
const VmaAllocationCreateInfo
get_allocation_create_info(const MemoryRequirements &requirements,
                           MemoryUsage usage, bool alias) noexcept;

} // namespace details

} // namespace strobe::rhi::vulkan
