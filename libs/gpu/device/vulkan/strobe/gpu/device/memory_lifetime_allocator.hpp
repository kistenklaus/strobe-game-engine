#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/memory_binding.hpp"
#include "strobe/gpu/device/memory_granularity_class.hpp"
#include "strobe/gpu/device/memory_lifetime.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/memory.hpp"
#include "strobe/gpu/vulkan/memory_requirements.hpp"
#include "strobe/gpu/vulkan/memory_usage.hpp"
#include <cstdlib>
#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>
namespace strobe::gpu {

class MemoryLifetimeAllocator {
public:
  using allocator = strobe::gpu::allocator_ref;

  struct MemoryBlockHeader {
    vulkan::Memory memory;
    VkDeviceSize size;
    uint32_t memoryTypeIndex;
  };

  struct ReserveInfo {
    VkDeviceSize alignment;
    uint32_t memoryTypeBits;
    uint8_t preferredMemoryTypeIndex;
    vulkan::MemoryUsage memoryUsage;
  };

  struct PlacementInfo {
    MemoryBlockHeader *block;
    VkDeviceSize offset;
  };

  enum class AllocationState : uint8_t {
    reserved,
    placed,
  };

  struct AllocationInfo {
    MemoryLifetime lifetime;
    VkDeviceSize size;
    uint32_t index;
    MemoryGranularityClass granularity;
    AllocationState state;
    union {
      ReserveInfo reservation;
      PlacementInfo placement;
    };
  };
  using allocation_info_pool =
      MonotonicPoolResource<sizeof(AllocationInfo), alignof(AllocationInfo),
                            allocator>;
  using info_pool_traits = AllocatorTraits<allocation_info_pool>;

  struct MemoryBlock : MemoryBlockHeader {
    Vector<AllocationInfo *> allocations;
    uint32_t index;
  };

  using memory_block_allocator =
      MonotonicPoolResource<sizeof(MemoryBlock), alignof(MemoryBlock),
                            allocator>;
  using block_alloc_traits = AllocatorTraits<memory_block_allocator>;

  struct Interval {
    AllocationInfo *info;
    VkDeviceSize offset;
  };

  struct MemoryUsagePolicy {
    VkMemoryPropertyFlags required;
    VkMemoryPropertyFlags prefered;
    VkMemoryPropertyFlags avoid;
  };

  MemoryLifetimeAllocator(const allocator &alloc)
      : m_alloc(alloc), m_infoPool(m_alloc), m_blockPool(m_alloc) {}

  MemoryLifetimeAllocator(const MemoryLifetimeAllocator &) = delete;
  MemoryLifetimeAllocator(MemoryLifetimeAllocator &&) = delete;
  MemoryLifetimeAllocator &operator=(const MemoryLifetimeAllocator &) = delete;
  MemoryLifetimeAllocator &operator=(MemoryLifetimeAllocator &&) = delete;
  ~MemoryLifetimeAllocator() noexcept {
    assert(m_liveAllocations.empty());
    assert(m_reservedAllocations.empty());
  }

  void *reserve_memory(vulkan::Context *ctx, const MemoryLifetime &lifetime,
                       const vulkan::MemoryRequirements &requirements,
                       vulkan::MemoryUsage memoryUsage,
                       MemoryGranularityClass granularity) {
    assert(requirements.size > 0);
    assert(requirements.alignment > 0);
    assert(std::has_single_bit(requirements.alignment));
    assert(requirements.memoryTypeBits != 0);
    assert(requirements.requiresDedicated == false);

    AllocationInfo *allocInfo =
        info_pool_traits::template allocate<AllocationInfo>(m_infoPool);

    std::construct_at(allocInfo);
    allocInfo->lifetime = lifetime;
    allocInfo->size = requirements.size;
    allocInfo->index = m_reservedAllocations.size();
    allocInfo->granularity = granularity;
    allocInfo->state = AllocationState::reserved;

    const VmaAllocationCreateInfo vmaInfo =
        vulkan::details::get_allocation_create_info(requirements, memoryUsage,
                                                    true);
    uint32_t preferredTypeIndex;
    vmaFindMemoryTypeIndex(ctx->vma(), requirements.memoryTypeBits, &vmaInfo,
                           &preferredTypeIndex);

    std::construct_at(&allocInfo->reservation,
                      ReserveInfo{
                          .alignment = requirements.alignment,
                          .memoryTypeBits = requirements.memoryTypeBits,
                          .preferredMemoryTypeIndex =
                              static_cast<uint8_t>(preferredTypeIndex),
                          .memoryUsage = memoryUsage,
                      });

    m_reservedAllocations.push_back(allocInfo);
    return allocInfo;
  }

  MemoryBinding commit_memory(vulkan::Context *ctx, void *internal) {
    auto *allocInfo = static_cast<AllocationInfo *>(internal);
    if (allocInfo->state == AllocationState::placed) {
      return MemoryBinding{
          allocInfo->placement.block->memory,
          allocInfo->placement.offset,
          allocInfo->size,
      };
    }

    auto [groupSize, memoryTypeBits] = maximum_preferred_anchor_grouping(
        allocInfo->index, m_reservedAllocations);

    std::span<AllocationInfo *> group{m_reservedAllocations.end() - groupSize,
                                      m_reservedAllocations.end()};
    Vector<VkDeviceSize> offsets{group.size()};
    VkDeviceSize size = greedy_pack_intervals(
        group, offsets,
        ctx->deviceInfo().properties.limits.bufferImageGranularity, m_alloc);

    MemoryBlock *block =
        alloc_memory_block(ctx,
                           vulkan::MemoryRequirements{
                               .size = size,
                               .alignment = 1,
                               .memoryTypeBits = memoryTypeBits,
                               .prefersDedicated = false,
                               .requiresDedicated = false,
                           },
                           vulkan::MemoryUsage::automatic);
    block->allocations.reserve(group.size());
    for (uint32_t i = 0; i < group.size(); ++i) {
      auto *info = group[i];
      std::destroy_at(&info->reservation);
      std::construct_at(&info->placement);
      info->placement.block = block;
      info->placement.offset = offsets[i];
      info->index = i;
      info->state = AllocationState::placed;
      block->allocations.push_back(info);
    }
    m_reservedAllocations.resize(m_reservedAllocations.size() - group.size());
    assert(allocInfo->state == AllocationState::placed);

    return MemoryBinding{
        block->memory,
        allocInfo->placement.offset,
        allocInfo->size,
    };
  }

  void commit_all(vulkan::Context *ctx) {
    while (!m_reservedAllocations.empty()) {
      commit_memory(ctx, m_reservedAllocations.front());
    }
  }

  void release_memory(vulkan::Context *ctx, void *internal) {
    assert(ctx != nullptr);
    assert(internal != nullptr);
    auto *allocInfo = static_cast<AllocationInfo *>(internal);
    switch (allocInfo->state) {
    case AllocationState::reserved: {
      const size_t index = allocInfo->index;
      AllocationInfo *moved = m_reservedAllocations.back();
      m_reservedAllocations[index] = moved;
      moved->index = index;
      m_reservedAllocations.pop_back();
      break;
    }
    case AllocationState::placed: {
      auto *block = static_cast<MemoryBlock *>(allocInfo->placement.block);
      const size_t index = allocInfo->index;
      AllocationInfo *moved = block->allocations.back();
      block->allocations[index] = moved;
      moved->index = index;
      block->allocations.pop_back();
      if (block->allocations.empty()) {
        free_memory_block(ctx, block);
      }
      break;
    }
    default:
      std::unreachable();
    }
    m_infoPool.deallocate(allocInfo);
  }

private:
  void free_memory_block(vulkan::Context *ctx, MemoryBlock *block) {
    assert(ctx != nullptr);
    assert(block != nullptr);
    assert(block->allocations.empty());
    assert(block->index < m_liveAllocations.size());
    assert(m_liveAllocations[block->index] == block);
    vulkan::free_memory(ctx, block->memory);
    const size_t index = block->index;
    MemoryBlock *moved = m_liveAllocations.back();
    m_liveAllocations[index] = moved;
    moved->index = static_cast<uint32_t>(index);
    m_liveAllocations.pop_back();
    std::destroy_at(block);
    m_blockPool.deallocate(block);
  }

  MemoryBlock *alloc_memory_block(vulkan::Context *ctx,
                                  vulkan::MemoryRequirements requirements,
                                  vulkan::MemoryUsage memoryUsage) {
    ZoneScopedN("allocate-memory-block");
    vulkan::Memory memory =
        vulkan::allocate_memory(ctx, requirements, memoryUsage, true);
    MemoryBlock *block =
        block_alloc_traits::template allocate<MemoryBlock>(m_blockPool);
    std::construct_at(block);
    block->memory = memory;
    block->size = requirements.size;
    block->memoryTypeIndex = requirements.memoryTypeBits;
    block->index = m_liveAllocations.size();
    m_liveAllocations.push_back(block);
    return block;
  }

  // Packs a compatible group of allocations into one virtual memory block.
  // Allocations with overlapping half-open lifetimes [begin, end) receive
  // disjoint ranges, while allocations with disjoint lifetimes may alias.
  // Linear and optimal resources are separated according to
  // bufferImageGranularity while simultaneously live.
  //
  // Uses a greedy alignment-aware line sweep; the result is valid but not
  // necessarily globally minimal. Expected time complexity is O(n log n),
  // with O(n) temporary memory obtained from alloc.
  static VkDeviceSize
  greedy_pack_intervals(std::span<AllocationInfo *> allocations,
                        std::span<VkDeviceSize> offsets,
                        VkDeviceSize bufferImageGranularity, allocator &alloc);

  static std::pair<size_t, uint32_t>
  greedy_preferred_grouping(uint32_t start,
                            std::span<AllocationInfo *> reserved);
  static std::pair<size_t, uint32_t>
  maximum_preferred_anchor_grouping(uint32_t start,
                                    std::span<AllocationInfo *> reserved);

private:
  [[no_unique_address]] allocator m_alloc;
  allocation_info_pool m_infoPool;
  memory_block_allocator m_blockPool;

  Vector<MemoryBlock *> m_liveAllocations;
  Vector<AllocationInfo *> m_reservedAllocations;
};

} // namespace strobe::gpu
