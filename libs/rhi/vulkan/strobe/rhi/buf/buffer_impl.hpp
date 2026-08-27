#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/memory/memory_allocation.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"

namespace strobe::rhi {

struct BufferImpl {
  BufferImpl(Context context, MemoryAllocation allocation,
             vulkan::Buffer buffer, VkDeviceSize size,
             VkDeviceAddress address) noexcept
      : context(std::move(context)), allocation(std::move(allocation)),
        buffer(buffer), size(size), address(address) {}

  BufferImpl(const BufferImpl &) = delete;
  BufferImpl(BufferImpl &&) = delete;
  BufferImpl &operator=(const BufferImpl &) = delete;
  BufferImpl &operator=(BufferImpl &&) = delete;
  ~BufferImpl() noexcept { vulkan::destroy_buffer(context.ctx(), buffer); }

  bool commit() {
    if (allocation.commit()) {
      vulkan::bind_buffer_memory(context.ctx(), allocation.binding().memory,
                                 buffer, allocation.binding().offset);
      address = vulkan::get_buffer_device_address(context.ctx(), buffer);
      return true;
    }
    return false;
  }

  bool is_bound() {
    return static_cast<bool>(allocation.binding());
  }

  const Context context;
  const MemoryAllocation allocation;
  const vulkan::Buffer buffer;
  const VkDeviceSize size;
  VkDeviceAddress address;
};
} // namespace strobe::rhi
