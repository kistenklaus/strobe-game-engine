#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_allocation_flags.hpp"
#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/memory/memory_binding.hpp"
#include "strobe/rhi/memory/memory_granularity_class.hpp"
#include "strobe/rhi/memory/memory_lifetime_allocator.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/memory/memory_requirements.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/types/memory_lifetime.hpp"
#include "strobe/rhi/utils/memory_usage_utils.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/image.hpp"
#include "strobe/rhi/vulkan/memory.hpp"
#include "strobe/rhi/vulkan/memory_requirements.hpp"
#include <fmt/printf.h>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct MemoryPoolImpl {

  using allocator = strobe::rhi::allocator;
  using allocator_ref = AllocatorReference<allocator>;
  using allocator_traits = AllocatorTraits<allocator>;

  Context context;
  std::mutex mutex;

  MemoryPoolImpl(Context context, const strobe::rhi::allocator_ref alloc)
      : context(std::move(context)), mutex{}, //
        m_lifetimeAllocator(alloc), m_handleAlloc{alloc} {}

  MemoryPoolImpl(const MemoryPool &) = delete;
  MemoryPoolImpl(MemoryPool &&) = delete;
  MemoryPoolImpl &operator=(const MemoryPool &) = delete;
  MemoryPoolImpl &operator=(MemoryPool &&) = delete;
  ~MemoryPoolImpl() noexcept {}

  std::pair<MemoryBinding, void *>
  allocate_memory(const MemoryRequirements &requirements,
                  const MemoryLifetime &lifetime) {
    ZoneScopedN("pool/allocate-memory");
    const vulkan::MemoryUsage memoryUsage =
        to_vulkan_memory_usage(requirements.memoryUsage);
    const vulkan::MemoryRequirements req{
        .size = requirements.size,
        .alignment = requirements.alignment,
        .memoryTypeBits = requirements.memoryTypeBits,
        .prefersDedicated =
            (requirements.flags & MemoryAllocationFlags::prefer_dedicated) != 0,
        .requiresDedicated = (requirements.flags &
                              MemoryAllocationFlags::require_dedicated) != 0,
    };

    if (req.prefersDedicated || req.requiresDedicated) {
      assert(!std::holds_alternative<std::monostate>(requirements.dedicated));

      if (std::holds_alternative<vulkan::Buffer>(requirements.dedicated)) {
        vulkan::Memory memory = vulkan::allocate_dedicated_memory(
            context.ctx(), req, memoryUsage,
            std::get<vulkan::Buffer>(requirements.dedicated));
        return std::make_pair(MemoryBinding{memory, 0, requirements.size},
                              nullptr);
      } else if (std::holds_alternative<vulkan::Image>(
                     requirements.dedicated)) {
        vulkan::Memory memory = vulkan::allocate_dedicated_memory(
            context.ctx(), req, memoryUsage,
            std::get<vulkan::Image>(requirements.dedicated));
        return std::make_pair(MemoryBinding{memory, 0, requirements.size},
                              nullptr);
      } else {
        throw std::runtime_error("invalid dedicated memory requirement");
      }
    }

    if (lifetime.begin == 0 &&
        lifetime.end == std::numeric_limits<decltype(lifetime.end)>::max()) {
      const vulkan::Memory memory =
          vulkan::allocate_memory(context.ctx(), req, memoryUsage, false);
      return std::make_pair(MemoryBinding{memory, 0, req.size}, nullptr);
    }
    void *internals = reserve_memory(lifetime, req, memoryUsage,
                                     requirements.granularityClass);

    return std::make_pair(MemoryBinding{{}, 0, requirements.size}, internals);
  }

  void commit_all() { m_lifetimeAllocator.commit_all(context.ctx()); }

  void free_memory(MemoryBinding memory, void *internal) {
    if (internal == nullptr) {
      assert(memory.offset == 0);
      vulkan::free_memory(context.ctx(), memory.memory);
    } else {
      release_memory(memory, internal);
    }
  }

  handle_allocator_ref<MemoryAllocationImpl> get_handle_alloc() noexcept {
    return &m_handleAlloc;
  }

  inline MemoryBinding commit_memory(void *internal) {
    assert(internal != nullptr);
    return m_lifetimeAllocator.commit_memory(context.ctx(), internal);
  }

private:
  inline void *reserve_memory(const MemoryLifetime &lifetime,
                              const vulkan::MemoryRequirements &requirements,
                              vulkan::MemoryUsage usage,
                              MemoryGranularityClass granularity) {
    ZoneScopedN("pool/reserve-memory");
    return m_lifetimeAllocator.reserve_memory(context.ctx(), lifetime,
                                              requirements, usage, granularity);
  }

  inline void release_memory([[maybe_unused]] MemoryBinding binding,
                             void *internal) {
    m_lifetimeAllocator.release_memory(context.ctx(), internal);
  }

  MemoryLifetimeAllocator m_lifetimeAllocator;
  handle_allocator<MemoryAllocationImpl> m_handleAlloc;

public:
  Buffer accelerationStructureScratch{};
};

} // namespace strobe::rhi
