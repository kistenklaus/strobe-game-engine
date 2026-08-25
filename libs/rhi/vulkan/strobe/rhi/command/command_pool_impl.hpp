#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/type_traits/unroll.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/command/command_buffer_handle_alloc.hpp"
#include "strobe/rhi/command/command_buffer_state_alloctor.hpp"
#include "strobe/rhi/command/native_command_pool.hpp"
#include "strobe/rhi/command/staging_buffer.hpp"
#include "strobe/rhi/device/context.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/command_pool.hpp"
#include "strobe/rhi/vulkan/memory.hpp"

#include <atomic>
#include <cassert>
#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct CommandPoolImpl {
  using allocator = strobe::rhi::allocator;
  using allocator_ref = AllocatorReference<allocator>;
  using allocator_traits = AllocatorTraits<allocator>;

  using state_allocator = cmd_buf_state_allocator;
  using state_allocator_ref = cmd_buf_state_allocator_ref;

  using handle_allocator = cmd_buf_handle_allocator;
  using handle_allocator_ref = cmd_buf_handle_allocator_ref;

  CommandPoolImpl(Context context, vulkan::Queue queue)
      : context(std::move(context)), queue(queue), m_alloc{},
        m_stateAlloc(&m_alloc), m_handleAlloc(&m_alloc) {}

  CommandPoolImpl(const CommandPoolImpl &) = delete;
  CommandPoolImpl(CommandPoolImpl &&) = delete;
  CommandPoolImpl &operator=(const CommandPoolImpl &) = delete;
  CommandPoolImpl &operator=(CommandPoolImpl &&) = delete;

  ~CommandPoolImpl() noexcept {
    destroy_pool_set(m_primary);
    destroy_pool_set(m_secondary);
    destroy_staging();
  }

  std::pair<NativeCommandPool *, vulkan::CommandBuffer> alloc(bool primary) {
    CmdPoolSet *set = primary ? &m_primary : &m_secondary;

    if (set->readyPools == nullptr) {
      set->readyPools =
          set->returnedPools.exchange(nullptr, std::memory_order_acquire);
      if (set->readyPools == nullptr) {
        vulkan::CommandPool pool = vulkan::create_command_pool(
            context.ctx(), {.queue = queue, .flags = 0});
        CmdPoolNode *node = allocator_traits::allocate<CmdPoolNode>(m_alloc);
        std::construct_at(node, pool, primary);
        node->next = nullptr;
        set->readyPools = node;
      }
    }

    assert(set->readyPools != nullptr);

    CmdPoolNode *node = set->readyPools;
    assert(!node->retired.load(std::memory_order_relaxed));

    vulkan::CommandBuffer cmd;

    if (!node->alloc(context.ctx(), &cmd)) {
      set->readyPools = node->next;
      node->next = nullptr;
      node->retired.store(true, std::memory_order_release);
    }

    assert(cmd);
    return {node, cmd};
  }

  void recycle(NativeCommandPool *pool) noexcept {
    if (!pool->recycle(context.ctx())) {
      return;
    }
    auto *node = static_cast<CmdPoolNode *>(pool);
    if (!node->retired.load(std::memory_order_acquire)) {
      return;
    }
    CmdPoolSet *set = node->primary() ? &m_primary : &m_secondary;
    node->retired.store(false, std::memory_order_relaxed);
    CmdPoolNode *head = set->returnedPools.load(std::memory_order_relaxed);
    do {
      node->next = head;
    } while (!set->returnedPools.compare_exchange_weak(
        head, node, std::memory_order_release, std::memory_order_relaxed));
  }

  state_allocator_ref get_state_allocator() noexcept { return &m_stateAlloc; }

  handle_allocator_ref get_handle_allocator() noexcept {
    return &m_handleAlloc;
  }

  const Context context;
  const vulkan::Queue queue;

private:
  [[no_unique_address]] allocator m_alloc;
  [[no_unique_address]] state_allocator m_stateAlloc;
  [[no_unique_address]] handle_allocator m_handleAlloc;

private: // CommandBuffer MPSC
  struct CmdPoolNode : NativeCommandPool {
    CmdPoolNode(vulkan::CommandPool cmdPool, bool primary)
        : NativeCommandPool{cmdPool, primary} {}
    CmdPoolNode *next = nullptr;
    std::atomic<bool> retired{false};
  };

  struct CmdPoolSet {
    CmdPoolNode *readyPools = nullptr;
    std::atomic<CmdPoolNode *> returnedPools{nullptr};
  };

  void destroy_pool_list(CmdPoolNode *node) noexcept {
    while (node != nullptr) {
      CmdPoolNode *next = node->next;

      assert(!node->retired.load(std::memory_order_relaxed));

      vulkan::destroy_command_pool(context.ctx(), node->native());

      std::destroy_at(node);
      allocator_traits::deallocate<CmdPoolNode>(m_alloc, node);

      node = next;
    }
  }
  void destroy_pool_set(CmdPoolSet &set) noexcept {
    CmdPoolNode *returned =
        set.returnedPools.exchange(nullptr, std::memory_order_relaxed);

    destroy_pool_list(set.readyPools);
    destroy_pool_list(returned);

    set.readyPools = nullptr;
  }

private:
  CmdPoolSet m_primary{};
  CmdPoolSet m_secondary{};

public: // StagingBuffer MPSC
  static constexpr std::array<VkDeviceSize, 5> STAGING_CLASSES{
      1 << 14, // 16KiB
      1 << 16, // 64KiB
      1 << 18, // 256KiB
      1 << 20, // 1MiB
      1 << 22, // 4MiB
               // larger (is not held persistantly)
  };

  // single consumer
  StagingBuffer *alloc_staging(VkDeviceSize minSize) {
    ZoneScopedN("pool/alloc-staging");
    size_t classIndex = STAGING_CLASSES.size();
    STROBE_UNROLL(STAGING_CLASSES.size())
    for (size_t i = 0; i < STAGING_CLASSES.size(); ++i) {
      if (minSize <= STAGING_CLASSES[i]) {
        classIndex = i;
        break;
      }
    }
    // Oversized allocations are never cached.
    if (classIndex == STAGING_CLASSES.size()) {
      return create_staging_buffer(minSize);
    }
    // Fast path
    if (StagingBuffer *buffer = m_readyStaging[classIndex]) {
      m_readyStaging[classIndex] = buffer->next;
      buffer->next = nullptr;
      return buffer;
    }
    // Pull m_returnedStaging
    StagingBuffer *returned =
        m_returnedStaging.exchange(nullptr, std::memory_order_acquire);
    while (returned != nullptr) {
      StagingBuffer *next = returned->next;
      returned->next = nullptr;
      size_t returnedClass = STAGING_CLASSES.size();
      STROBE_UNROLL(STAGING_CLASSES.size())
      for (size_t i = 0; i < STAGING_CLASSES.size(); ++i) {
        if (returned->size <= STAGING_CLASSES[i]) {
          returnedClass = i;
          break;
        }
      }
      if (returnedClass == STAGING_CLASSES.size()) {
        // Oversized staging buffers are not retained.
        destroy_staging_buffer(returned);
      } else {
        returned->next = m_readyStaging[returnedClass];
        m_readyStaging[returnedClass] = returned;
      }
      returned = next;
    }

    // Something of the requested class may have been returned.
    if (StagingBuffer *buffer = m_readyStaging[classIndex]) {
      m_readyStaging[classIndex] = buffer->next;
      buffer->next = nullptr;
      return buffer;
    }
    return create_staging_buffer(STAGING_CLASSES[classIndex]);
  }

  // multiple producer
  void recycle_staging(StagingBuffer *begin, StagingBuffer *end) noexcept {
    if (begin == nullptr) {
      assert(end == nullptr);
      return;
    }
    assert(end != nullptr);
    StagingBuffer *head = m_returnedStaging.load(std::memory_order_relaxed);
    do {
      end->next = head;
    } while (!m_returnedStaging.compare_exchange_weak(
        head, begin, std::memory_order_release, std::memory_order_relaxed));
  }

private:
  StagingBuffer *create_staging_buffer(VkDeviceSize size) {
    auto [memory, buffer, _] = vulkan::allocate_buffer(
        context.ctx(),
        {.size = size, .usage = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT},
        vulkan::MemoryUsage::mapped_write_sequential);
    ZoneScopedN("pool/create-staging-buffer");
    vulkan::bind_buffer_memory(context.ctx(), memory, buffer, 0);
    void *mapped = vulkan::map_memory(context.ctx(), memory);
    assert(mapped);
    auto *stage = allocator_traits::template allocate<StagingBuffer>(m_alloc);
    std::construct_at(stage, buffer, memory, size, mapped);
    return stage;
  }

  void destroy_staging_buffer(StagingBuffer *buffer) {
    vulkan::unmap_memory(context.ctx(), buffer->memory);
    vulkan::free_allocated_buffer(context.ctx(), buffer->memory,
                                  buffer->buffer);
    allocator_traits::template deallocate<StagingBuffer>(m_alloc, buffer);
  }

  void destroy_staging_list(StagingBuffer *buffer) noexcept {
    while (buffer != nullptr) {
      StagingBuffer *next = buffer->next;
      destroy_staging_buffer(buffer);
      buffer = next;
    }
  }

  // destructor helpers.
  void destroy_staging() noexcept {
    StagingBuffer *returned =
        m_returnedStaging.exchange(nullptr, std::memory_order_relaxed);

    for (StagingBuffer *&ready : m_readyStaging) {
      destroy_staging_list(ready);
      ready = nullptr;
    }

    destroy_staging_list(returned);
  }

private:
  std::array<StagingBuffer *, STAGING_CLASSES.size()> m_readyStaging{};
  std::atomic<StagingBuffer *> m_returnedStaging{nullptr};
};

} // namespace strobe::rhi
