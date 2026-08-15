#pragma once

#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/vulkan/buffer.hpp"

namespace strobe::gpu {

struct BufferImpl {
  BufferImpl(Context context, vulkan::Buffer buffer, VkDeviceSize size,
             VkDeviceAddress address) noexcept
      : context(std::move(context)), buffer(buffer), size(size),
        address(address) {}
  BufferImpl(const BufferImpl &) = delete;
  BufferImpl(BufferImpl &&) = delete;
  BufferImpl &operator=(const BufferImpl &) = delete;
  BufferImpl &operator=(BufferImpl &&) = delete;
  ~BufferImpl() noexcept { vulkan::destroy_buffer(context.get(), buffer); }

  const Context context;
  const vulkan::Buffer buffer;
  const VkDeviceSize size;
  const VkDeviceAddress address;
};
} // namespace strobe::gpu
