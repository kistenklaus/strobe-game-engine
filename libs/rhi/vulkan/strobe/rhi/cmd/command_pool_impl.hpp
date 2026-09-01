#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/cmd/command_buffer_state_alloctor.hpp"
#include "strobe/rhi/cmd/native_command_pool.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/vulkan/command_pool.hpp"

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

  CommandPoolImpl(Context context, StagingPool stagingPool,
                  uint32_t queueFamily, cmd_buf_state_allocator_ref stateAlloc,
                  strobe::rhi::allocator_ref alloc) noexcept 
      : context(std::move(context)), stagingPool(std::move(stagingPool)),
        queueFamily(queueFamily),
#ifdef STROBE_TRACY
        profilerContext(this->context.ctx()),
#endif
        stateAlloc(stateAlloc), cmdAlloc(alloc), m_nodePool(alloc) {
  }

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
            context.ctx(), {.queueFamily = queueFamily, .flags = 0});
        CmdPoolNode *node = static_cast<CmdPoolNode *>(m_nodePool.allocate());
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
      m_nodePool.deallocate(node);
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

public:
  const Context context;
  const StagingPool stagingPool;
  const uint32_t queueFamily;
#ifdef STROBE_TRACY
  [[maybe_unused]] strobe::rhi::profiler::Context profilerContext;
#endif

  cmd_buf_state_allocator_ref stateAlloc;
  handle_allocator<CommandBufferImpl> cmdAlloc;

private:
  MPSCMonotonicPoolResource<sizeof(CmdPoolNode), alignof(CmdPoolNode),
                            strobe::rhi::allocator_ref>
      m_nodePool;
  CmdPoolSet m_primary{};
  CmdPoolSet m_secondary{};
};

} // namespace strobe::rhi
