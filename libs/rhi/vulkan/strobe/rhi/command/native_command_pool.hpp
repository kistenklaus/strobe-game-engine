#pragma once

#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/command_pool.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct NativeCommandPool {
  static constexpr size_t CAPACITY = 32;

  NativeCommandPool(vulkan::CommandPool pool, bool primary) noexcept
      : m_pool(pool), m_primary(primary) {}
  NativeCommandPool(const NativeCommandPool &) = delete;
  NativeCommandPool(NativeCommandPool &&) = delete;
  NativeCommandPool &operator=(const NativeCommandPool &) = delete;
  NativeCommandPool &operator=(NativeCommandPool &&) = delete;
  ~NativeCommandPool() noexcept { assert(m_live == 0); }

  bool alloc(vulkan::Context *context, vulkan::CommandBuffer *cmd) {
    std::lock_guard lock{m_mutex};
    assert(m_size < CAPACITY);
    if (m_size == m_allocated) {
      bulk_alloc(context);
    }
    assert(m_size < m_allocated);
    const uint32_t index = m_size++;
    m_live.fetch_add(1, std::memory_order_relaxed);
    *cmd = vulkan::CommandBuffer{
        .handle = m_cmds[index],
    };
    return m_size < CAPACITY;
  }

  // may be called concurrently
  bool recycle(vulkan::Context *context) {
    const uint32_t prev = m_live.fetch_sub(1, std::memory_order_relaxed);
    assert(prev != 0);
    if (prev != 1) {
      return false;
    }
    std::lock_guard lock{m_mutex};
    if (m_live.load(std::memory_order_relaxed) != 0) {
      return false;
    }
    reset(context);
    return true;
  }

  bool primary() const noexcept { return m_primary; }

  vulkan::CommandPool native() const noexcept { return m_pool; }

private:
  void bulk_alloc(vulkan::Context *context) {
    assert(m_allocated <= (CAPACITY / 2));

    const uint32_t new_allocated = m_allocated != 0 ? (2 * m_allocated) : 1;
    const uint32_t alloc_count = new_allocated - m_allocated;
    [[maybe_unused]] static constexpr size_t MAX_ALLOC_COUNT = CAPACITY / 2;
    assert(alloc_count <= MAX_ALLOC_COUNT);

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_pool.handle,
        .level = m_primary ? (VK_COMMAND_BUFFER_LEVEL_PRIMARY)
                           : (VK_COMMAND_BUFFER_LEVEL_SECONDARY),
        .commandBufferCount = alloc_count,
    };

    {
      ZoneScopedN("vkAllocateCommandBuffers");
      VkResult result = vkAllocateCommandBuffers(context->device(), &allocInfo,
                                                 m_cmds.data() + m_allocated);
      if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
      }
    }
    m_allocated = new_allocated;
  }

  void reset(vulkan::Context *context) {
    assert(m_live == 0);
    vulkan::reset_command_pool(context, m_pool);
    m_size = 0;
  }

  vulkan::CommandPool m_pool;
  const bool m_primary;

  uint32_t m_allocated{0};
  uint32_t m_size{0}; // <- size of state (vec)
  std::array<VkCommandBuffer, CAPACITY> m_cmds;

  std::atomic<uint32_t> m_live{0}; // <- amount of outstanding allocations.
  std::mutex m_mutex;
};

} // namespace strobe::rhi
