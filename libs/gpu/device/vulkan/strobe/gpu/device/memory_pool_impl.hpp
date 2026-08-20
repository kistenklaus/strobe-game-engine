#pragma once

#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/buffer_handle_alloc.hpp"
#include "strobe/gpu/device/buffer_usage_utils.hpp"
#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/memory_allocation_handle_alloc.hpp"
#include "strobe/gpu/device/memory_binding.hpp"
#include "strobe/gpu/device/memory_granularity_class.hpp"
#include "strobe/gpu/device/memory_lifetime.hpp"
#include "strobe/gpu/device/memory_lifetime_allocator.hpp"
#include "strobe/gpu/device/memory_pool.hpp"
#include "strobe/gpu/device/memory_usage_utils.hpp"
#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/memory.hpp"
#include "strobe/gpu/vulkan/memory_requirements.hpp"
#include <limits>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

struct MemoryPoolImpl {

  using allocator = strobe::gpu::allocator;
  using allocator_ref = AllocatorReference<allocator>;
  using allocator_traits = AllocatorTraits<allocator>;

  using handle_allocator = memory_allocation_handle_allocator;
  using handle_allocator_ref = memory_allocation_handle_allocator_ref;

  MemoryPoolImpl(Context context)
      : context(std::move(context)), m_alloc{}, m_lifetimeAllocator(&m_alloc),
        m_handleAlloc{std::in_place, &m_alloc}, m_bufferHandelAlloc{std::in_place, &m_alloc} {}
  MemoryPoolImpl(const MemoryPool &) = delete;
  MemoryPoolImpl(MemoryPool &&) = delete;
  MemoryPoolImpl &operator=(const MemoryPool &) = delete;
  MemoryPoolImpl &operator=(MemoryPool &&) = delete;
  ~MemoryPoolImpl() noexcept = default;

  std::tuple<MemoryBinding, vulkan::Buffer, void *>
  create_buffer(const BufferCreateInfo &info, const MemoryLifetime &lifetime) {
    ZoneScopedN("pool/create-buffer");
    const vulkan::MemoryUsage memoryUsage =
        to_vulkan_memory_usage(info.memory_usage);
    const VkBufferUsageFlags2 usage =
        to_vk_buffer_usage(info.usage) |
        VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    if (lifetime.begin == 0 &&
        lifetime.end == std::numeric_limits<uint32_t>::max()) {
      // Non-Lifetime
      auto [memory, buffer] = vulkan::allocate_buffer(context.get(),
                                                      {
                                                          .size = info.size,
                                                          .usage = usage,
                                                      },
                                                      memoryUsage);
      vulkan::bind_buffer_memory(context.get(), memory, buffer, 0);
      return std::make_tuple(MemoryBinding{memory, 0}, buffer, nullptr);
    }
    vulkan::Buffer buffer = vulkan::create_buffer(
        context.get(), {.size = info.size, .usage = usage});
    vulkan::MemoryRequirements requirements =
        vulkan::get_buffer_memory_requirements(context.get(), buffer);

    if (requirements.prefersDedicated || requirements.requiresDedicated) {
      // Dedicated allocation
      vulkan::Memory memory = vulkan::allocate_memory(
          context.get(), requirements, memoryUsage, false);
      vulkan::bind_buffer_memory(context.get(), memory, buffer, 0);
      return std::make_tuple(MemoryBinding{memory, 0}, buffer, nullptr);
    }
    // Lifetime & Non-Dedicated allocation -> internally managed
    void *internals = reserve_memory(lifetime, requirements, memoryUsage,
                                     MemoryGranularityClass::linear);
    return std::make_tuple(MemoryBinding{{}, 0}, buffer, internals);
  }

  MemoryBinding bind_buffer(vulkan::Buffer buffer, void *internals) {
    MemoryBinding binding = commit_memory(internals);
    vulkan::bind_buffer_memory(context.get(), binding.memory, buffer,
                               binding.offset);
    return binding;
  }

  void commit_all() { m_lifetimeAllocator.commit_all(context.get()); }

  void free_memory(MemoryBinding memory, void *internal) {
    if (internal == nullptr) {
      assert(memory.offset == 0);
      vulkan::free_memory(context.get(), memory.memory);
    } else {
      release_memory(memory, internal);
    }
  }

  Context context;

  memory_allocation_handle_allocator_ref get_handle_alloc() noexcept {
    return &m_handleAlloc;
  }

  buffer_handle_alloc_ref get_buffer_handle_alloc() noexcept {
    return &m_bufferHandelAlloc;
  }

private:
  inline void *reserve_memory(const MemoryLifetime &lifetime,
                              const vulkan::MemoryRequirements &requirements,
                              vulkan::MemoryUsage usage,
                              MemoryGranularityClass granularity) {
    ZoneScopedN("pool/reserve-memory");
    return m_lifetimeAllocator.reserve_memory(context.get(), lifetime,
                                              requirements, usage, granularity);
  }

  inline MemoryBinding commit_memory(void *internal) {
    return m_lifetimeAllocator.commit_memory(context.get(), internal);
  }

  inline void release_memory([[maybe_unused]] MemoryBinding binding,
                             void *internal) {
    m_lifetimeAllocator.release_memory(context.get(), internal);
  }

  [[no_unique_address]] allocator m_alloc;
  MemoryLifetimeAllocator m_lifetimeAllocator;
  memory_allocation_handle_allocator m_handleAlloc;
  buffer_handle_alloc m_bufferHandelAlloc;
};

} // namespace strobe::gpu
