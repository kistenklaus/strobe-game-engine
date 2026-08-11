#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.h>

namespace strobe::gpu::vulkan {

#if (defined(TRACY_ENABLE) && defined(STROBE_INSTRUMENT_VULKAN_DRIVER))

class DriverAlloc {
private:
  struct alloc_header {
    void *base;
    std::size_t allocation_size;
    std::size_t requested_size;
    std::size_t allocation_alignment;
  };

public:
  using allocator = strobe::Mallocator;
  using allocator_traits = AllocatorTraits<allocator>;

  explicit DriverAlloc(const allocator &upstream = {}) : m_upstream(upstream) {
    TracyPlotConfig("vulkan-driver", tracy::PlotFormatType::Memory, true, true,
                    0);

    m_callbacks = VkAllocationCallbacks{
        .pUserData = this,
        .pfnAllocation = &allocation_callback,
        .pfnReallocation = &reallocation_callback,
        .pfnFree = &free_callback,
        .pfnInternalAllocation = &internal_allocation_callback,
        .pfnInternalFree = &internal_free_callback,
    };
  }
  DriverAlloc(const DriverAlloc &) = delete;
  DriverAlloc &operator=(const DriverAlloc &) = delete;

  DriverAlloc(DriverAlloc &&) = delete;
  DriverAlloc &operator=(DriverAlloc &&) = delete;

  inline __attribute((always_inline)) const VkAllocationCallbacks *
  callbacks() const noexcept {
    return &m_callbacks;
  }

private:
  [[nodiscard]]
  void *allocate(std::size_t size, std::size_t alignment) noexcept {
    if (size == 0) {
      return nullptr;
    }

    alignment = std::max(alignment, alignof(alloc_header));

    if ((alignment & (alignment - 1)) != 0) {
      return nullptr;
    }

    constexpr std::size_t header_size = sizeof(alloc_header);

    if (size > std::numeric_limits<std::size_t>::max() - header_size -
                   (alignment - 1)) {
      return nullptr;
    }

    const std::size_t allocation_size = size + header_size + alignment - 1;

    constexpr std::size_t allocation_alignment = alignof(std::max_align_t);

    void *base = allocator_traits::allocate(m_upstream, allocation_size,
                                            allocation_alignment);

    if (base == nullptr) {
      return nullptr;
    }

    const auto begin = reinterpret_cast<std::uintptr_t>(base) + header_size;

    const auto aligned =
        (begin + alignment - 1) & ~(static_cast<std::uintptr_t>(alignment) - 1);

    auto *header = reinterpret_cast<alloc_header *>(aligned - header_size);

    *header = {
        .base = base,
        .allocation_size = allocation_size,
        .requested_size = size,
        .allocation_alignment = allocation_alignment,
    };

    void *ptr = reinterpret_cast<void *>(aligned);
    TracyAllocN(ptr, size, "vulkan-context");
    return ptr;
  }

  void deallocate(void *ptr) noexcept {
    if (ptr == nullptr) {
      return;
    }

    auto *header = get_header(ptr);

    TracyFreeN(ptr, "vulkan-context");
    allocator_traits::deallocate(m_upstream, header->base,
                                 header->allocation_size,
                                 header->allocation_alignment);
  }

  [[nodiscard]]
  void *reallocate(void *original, std::size_t size,
                   std::size_t alignment) noexcept {
    if (original == nullptr) {
      return allocate(size, alignment);
    }

    if (size == 0) {
      deallocate(original);
      return nullptr;
    }

    const std::size_t old_size = get_header(original)->requested_size;

    void *replacement = allocate(size, alignment);

    if (replacement == nullptr) {
      return nullptr;
    }

    std::memcpy(replacement, original, std::min(old_size, size));

    deallocate(original);

    return replacement;
  }

  void internal_allocate(std::size_t size, VkInternalAllocationType,
                         VkSystemAllocationScope) noexcept {
    const auto total =
        m_internal_bytes.fetch_add(static_cast<std::int64_t>(size),
                                   std::memory_order_relaxed) +
        static_cast<std::int64_t>(size);

    TracyPlot("vulkan-driver", total);
  }

  void internal_free(std::size_t size, VkInternalAllocationType,
                     VkSystemAllocationScope) noexcept {
    const auto total =
        m_internal_bytes.fetch_sub(static_cast<std::int64_t>(size),
                                   std::memory_order_relaxed) -
        static_cast<std::int64_t>(size);

    TracyPlot("vulkan-driver", total);
  }

  [[nodiscard]]
  static alloc_header *get_header(void *ptr) noexcept {
    return reinterpret_cast<alloc_header *>(static_cast<std::byte *>(ptr) -
                                            sizeof(alloc_header));
  }

  static void *VKAPI_PTR allocation_callback(void *user_data, std::size_t size,
                                             std::size_t alignment,
                                             VkSystemAllocationScope) noexcept {
    return static_cast<DriverAlloc *>(user_data)->allocate(size, alignment);
  }

  static void *VKAPI_PTR reallocation_callback(
      void *user_data, void *original, std::size_t size, std::size_t alignment,
      VkSystemAllocationScope) noexcept {
    return static_cast<DriverAlloc *>(user_data)->reallocate(original, size,
                                                             alignment);
  }

  static void VKAPI_PTR free_callback(void *user_data, void *memory) noexcept {
    static_cast<DriverAlloc *>(user_data)->deallocate(memory);
  }
  static void VKAPI_PTR internal_allocation_callback(
      void *user_data, std::size_t size, VkInternalAllocationType type,
      VkSystemAllocationScope scope) noexcept {
    auto &self = *static_cast<DriverAlloc *>(user_data);

    self.internal_allocate(size, type, scope);
  }

  static void VKAPI_PTR internal_free_callback(
      void *user_data, std::size_t size, VkInternalAllocationType type,
      VkSystemAllocationScope scope) noexcept {
    auto &self = *static_cast<DriverAlloc *>(user_data);

    self.internal_free(size, type, scope);
  }

private:
  [[no_unique_address]]
  allocator m_upstream;

  std::atomic<std::int64_t> m_internal_bytes = 0;
  VkAllocationCallbacks m_callbacks;
};

#else

class DriverAlloc {
public:
  using allocator = std::remove_cvref_t<strobe::Mallocator>;
  using allocator_traits = AllocatorTraits<allocator>;

  explicit DriverAlloc([[maybe_unused]] const allocator &upstream = {}) {}
  DriverAlloc(const DriverAlloc &) = delete;
  DriverAlloc &operator=(const DriverAlloc &) = delete;

  DriverAlloc(DriverAlloc &&) = delete;
  DriverAlloc &operator=(DriverAlloc &&) = delete;
  inline __attribute((always_inline)) VkAllocationCallbacks *
  callbacks() const noexcept {
    return nullptr;
  }
};

#endif

} // namespace strobe::gpu::vulkan
