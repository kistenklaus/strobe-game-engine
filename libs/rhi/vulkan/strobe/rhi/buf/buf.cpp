#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/memory/memory_allocation_flags.hpp"
#include "strobe/rhi/memory/memory_granularity_class.hpp"
#include "strobe/rhi/utils/buffer_usage_utils.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"

namespace strobe::rhi::buf {

Buffer create_buffer(const MemoryPool &memoryPool, const BufferInfo &info,
                     const MemoryLifetime &lifetime, handle_allocators *alloc) {

  Context context = memoryPool.context();
  vulkan::Context *ctx = context.ctx();
  vulkan::Buffer buffer = vulkan::create_buffer(
      ctx, {.size = info.size, .usage = to_vk_buffer_usage(info.bufferUsage)});

  vulkan::MemoryRequirements requirements =
      vulkan::get_buffer_memory_requirements(ctx, buffer);

  MemoryAllocationFlags flags{};
  if (requirements.prefersDedicated) {
    flags |= MemoryAllocationFlags::prefer_dedicated;
  }
  if (requirements.requiresDedicated) {
    flags |= MemoryAllocationFlags::require_dedicated;
  }

  MemoryAllocation allocation = memoryPool.allocate_memory(
      {
          .size = requirements.size,
          .alignment = requirements.alignment,
          .memoryTypeBits = requirements.memoryTypeBits,
          .granularityClass = MemoryGranularityClass::linear,
          .flags = flags,
          .memoryUsage = info.memoryUsage,
          .dedicated = buffer,
      },
      lifetime);
  VkDeviceAddress address = 0;
  if (allocation.binding()) {
    vulkan::bind_buffer_memory(ctx, allocation.binding().memory, buffer,
                               allocation.binding().offset);
    address = vulkan::get_buffer_device_address(ctx, buffer);
  }

  return Buffer{make_void_handle<BufferImpl>(
      &alloc->bufferAllocator, std::move(context), std::move(allocation),
      buffer, info.size, address)};
}
} // namespace strobe::rhi::buf
