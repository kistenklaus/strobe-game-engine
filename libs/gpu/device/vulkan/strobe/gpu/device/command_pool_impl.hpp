#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/command_buffer_handle_alloc.hpp"
#include "strobe/gpu/device/command_buffer_state_alloctor.hpp"
#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/native_command_pool.hpp"
#include "strobe/gpu/vulkan/command_pool.hpp"

#include <atomic>
#include <cassert>
#include <utility>

namespace strobe::gpu {

struct CommandPoolImpl {
  using allocator = strobe::gpu::allocator;
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
  }

  std::pair<NativeCommandPool *, vulkan::CommandBuffer> alloc(bool primary) {
    CmdPoolSet *set = primary ? &m_primary : &m_secondary;

    if (set->readyPools == nullptr) {
      set->readyPools =
          set->returnedPools.exchange(nullptr, std::memory_order_acquire);
      if (set->readyPools == nullptr) {
        vulkan::CommandPool pool = vulkan::create_command_pool(
            context.get(), {.queue = queue, .flags = 0});
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

    if (!node->alloc(context.get(), &cmd)) {
      set->readyPools = node->next;
      node->next = nullptr;
      node->retired.store(true, std::memory_order_release);
    }

    assert(cmd);
    return {node, cmd};
  }

  void recycle(NativeCommandPool *pool) noexcept {
    if (!pool->recycle(context.get())) {
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

private:
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

      vulkan::destroy_command_pool(context.get(), node->native());

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
};

} // namespace strobe::gpu
