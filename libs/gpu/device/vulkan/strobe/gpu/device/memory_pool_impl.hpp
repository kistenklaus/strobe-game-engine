#pragma once

#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/buffer_handle_alloc.hpp"
#include "strobe/gpu/device/buffer_usage_utils.hpp"
#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/format_utilts.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/image_handle_alloc.hpp"
#include "strobe/gpu/device/image_type_utils.hpp"
#include "strobe/gpu/device/image_usage_utils.hpp"
#include "strobe/gpu/device/memory_allocation_handle_alloc.hpp"
#include "strobe/gpu/device/memory_binding.hpp"
#include "strobe/gpu/device/memory_granularity_class.hpp"
#include "strobe/gpu/device/memory_lifetime.hpp"
#include "strobe/gpu/device/memory_lifetime_allocator.hpp"
#include "strobe/gpu/device/memory_pool.hpp"
#include "strobe/gpu/device/memory_usage_utils.hpp"
#include "strobe/gpu/device/sample_count_utils.hpp"
#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/image.hpp"
#include "strobe/gpu/vulkan/memory.hpp"
#include "strobe/gpu/vulkan/memory_requirements.hpp"
#include <limits>
#include <tuple>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

struct MemoryPoolImpl {
  static constexpr uint64_t MAX_SCRATCH_SIZE = 1 << 28; // 256MiB

  using allocator = strobe::gpu::allocator;
  using allocator_ref = AllocatorReference<allocator>;
  using allocator_traits = AllocatorTraits<allocator>;

  using handle_allocator = memory_allocation_handle_allocator;
  using handle_allocator_ref = memory_allocation_handle_allocator_ref;

  MemoryPoolImpl(Context context)
      : context(std::move(context)), m_alloc{}, m_lifetimeAllocator(&m_alloc),
        m_handleAlloc{std::in_place, &m_alloc},
        m_bufferHandelAlloc{std::in_place, &m_alloc},
        m_imageHandleAlloc{std::in_place, &m_alloc} {}

  MemoryPoolImpl(const MemoryPool &) = delete;
  MemoryPoolImpl(MemoryPool &&) = delete;
  MemoryPoolImpl &operator=(const MemoryPool &) = delete;
  MemoryPoolImpl &operator=(MemoryPool &&) = delete;
  ~MemoryPoolImpl() noexcept { fmt::println("mem-pool-destructor"); }

  std::tuple<MemoryBinding, vulkan::Buffer, void *>
  create_buffer(const BufferInfo &info, const MemoryLifetime &lifetime) {
    ZoneScopedN("pool/create-buffer");
    const vulkan::MemoryUsage memoryUsage =
        to_vulkan_memory_usage(info.memoryUsage);
    const VkBufferUsageFlags2 usage =
        to_vk_buffer_usage(info.bufferUsage) |
        VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    if (lifetime.begin == 0 &&
        lifetime.end == std::numeric_limits<uint32_t>::max()) {
      // Non-Lifetime
      auto [memory, buffer, size] =
          vulkan::allocate_buffer(context.get(),
                                  {
                                      .size = info.size,
                                      .usage = usage,
                                  },
                                  memoryUsage);
      vulkan::bind_buffer_memory(context.get(), memory, buffer, 0);
      return std::make_tuple(MemoryBinding{memory, 0, size}, buffer, nullptr);
    }
    vulkan::Buffer buffer = vulkan::create_buffer(
        context.get(), {.size = info.size, .usage = usage});
    vulkan::MemoryRequirements requirements =
        vulkan::get_buffer_memory_requirements(context.get(), buffer);

    if (requirements.prefersDedicated || requirements.requiresDedicated) {
      // Dedicated allocation
      vulkan::Memory memory = vulkan::allocate_dedicated_memory(
          context.get(), requirements, memoryUsage, buffer);
      vulkan::bind_buffer_memory(context.get(), memory, buffer, 0);
      return std::make_tuple(MemoryBinding{memory, 0, requirements.size},
                             buffer, nullptr);
    }
    // Lifetime & Non-Dedicated allocation -> internally managed
    void *internals = reserve_memory(lifetime, requirements, memoryUsage,
                                     MemoryGranularityClass::linear);
    return std::make_tuple(MemoryBinding{{}, 0, requirements.size}, buffer,
                           internals);
  }

  std::tuple<MemoryBinding, vulkan::Image, void *>
  create_image(const ImageCreateInfo &info, const MemoryLifetime &lifetime) {
    ZoneScopedN("pool/create-image");
    const vulkan::MemoryUsage memoryUsage =
        to_vulkan_memory_usage(info.memoryUsage);

    vulkan::ImageInfo imageInfo{
        .type = to_vk_image_type(info.type),
        .format = to_vk_format(info.format),
        .extent =
            VkExtent3D{
                .width = info.extent.x(),
                .height = info.extent.y(),
                .depth = info.extent.z(),
            },
        .mip_levels = info.mip_levels,
        .array_layers = info.arrayLayers,
        .samples = to_vk_sample_count(info.samples),
        .tiling = info.linearTiling ? VK_IMAGE_TILING_LINEAR
                                    : VK_IMAGE_TILING_OPTIMAL,
        .usage = to_vk_image_usage(info.usage),
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    if (lifetime.begin == 0 &&
        lifetime.end == std::numeric_limits<uint32_t>::max()) {
      auto [memory, image, size] =
          vulkan::allocate_image(context.get(), imageInfo, memoryUsage);

      vulkan::bind_image_memory(context.get(), memory, image, 0);
      return std::make_tuple(MemoryBinding{memory, 0, size}, image, nullptr);
    }
    vulkan::Image image = vulkan::create_image(context.get(), imageInfo);
    vulkan::MemoryRequirements requirements =
        vulkan::get_image_memory_requirements(context.get(), image);
    if (requirements.prefersDedicated || requirements.requiresDedicated) {
      vulkan::Memory memory = vulkan::allocate_dedicated_memory(
          context.get(), requirements, memoryUsage, image);
      vulkan::bind_image_memory(context.get(), memory, image, 0);
      return std::make_tuple(MemoryBinding{memory, 0, requirements.size}, image,
                             nullptr);
    }

    void *internals =
        reserve_memory(lifetime, requirements, memoryUsage,
                       info.linearTiling ? MemoryGranularityClass::linear
                                         : MemoryGranularityClass::optimal);
    return std::make_tuple(MemoryBinding{{}, 0, requirements.size}, image,
                           internals);
  }

  MemoryBinding bind_buffer(vulkan::Buffer buffer, void *internals) {
    MemoryBinding binding = commit_memory(internals);
    vulkan::bind_buffer_memory(context.get(), binding.memory, buffer,
                               binding.offset);
    return binding;
  }

  MemoryBinding bind_image(vulkan::Image image, void *internals) {
    MemoryBinding binding = commit_memory(internals);
    vulkan::bind_image_memory(context.get(), binding.memory, image,
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

  image_handle_alloc_ref get_image_handle_alloc() noexcept {
    return &m_imageHandleAlloc;
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
  image_handle_alloc m_imageHandleAlloc;

public:
  Buffer accelerationStructureScratch{};
};

} // namespace strobe::gpu
