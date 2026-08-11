#pragma once

#include <vk_mem_alloc.h>

namespace strobe::gpu::vulkan {

enum class MemoryUsage {
  automatic,
  device, // require DEVICE_LOCAL
  mapped, // HOST_VISIBLE, persistent mapping, CPU reads/random access
  mapped_write_sequential, // HOST_VISIBLE, persistent mapping, sequential CPU
                           // writes
};

namespace details {

[[nodiscard]]
const VmaAllocationCreateInfo *get_allocation_create_info(MemoryUsage usage) noexcept;

} // namespace details

} // namespace strobe::gpu::vulkan
