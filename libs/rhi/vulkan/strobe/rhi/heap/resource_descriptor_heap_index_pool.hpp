#pragma once

#include "strobe/core/containers/bitmap_index_pool.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/vulkan/device_info/device_properties.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <mutex>

namespace strobe::rhi {

class ResourceDescriptorHeapIndexPool {
public:
  static constexpr uint32_t INVALID_INDEX =
      std::numeric_limits<uint32_t>::max();

private:
  static constexpr uint64_t
  unified_stride(const vulkan::DescriptorHeapProperties &properties) noexcept {
    return std::max<uint64_t>(properties.bufferDescriptorSize,
                              properties.imageDescriptorSize);
  }

  static constexpr uint32_t
  slot_count(uint64_t bufferSize,
             const vulkan::DescriptorHeapProperties &properties) noexcept {
    assert(bufferSize >= properties.minResourceHeapReservedRange);

    if (bufferSize < properties.minResourceHeapReservedRange) {
      return 0;
    }
    const uint64_t stride = unified_stride(properties);
    assert(stride != 0);
    if (stride == 0) {
      return 0;
    }
    const uint64_t effectiveSize =
        bufferSize - properties.minResourceHeapReservedRange;
    const uint64_t slotCount = effectiveSize / stride;
    assert(slotCount <= std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>(slotCount);
  }

public:
  explicit ResourceDescriptorHeapIndexPool(
      uint64_t bufferSize, const vulkan::DescriptorHeapProperties &properties,
      strobe::rhi::allocator_ref alloc) noexcept
      : m_stride(unified_stride(properties)),
        m_minReservedRange(properties.minResourceHeapReservedRange),
        m_maxBufferSize(properties.maxResourceHeapSize),
        m_bufferSize(bufferSize),
        m_descriptorCount(slot_count(bufferSize, properties)),
        m_indexPool(m_descriptorCount, alloc) {
    assert(bufferSize <= m_maxBufferSize);
    assert((descriptor_region_size_unlocked() %
            properties.bufferDescriptorAlignment) == 0);
    assert((descriptor_region_size_unlocked() %
            properties.imageDescriptorAlignment) == 0);
  }

  uint32_t alloc_buffer(uint32_t count = 1) noexcept {
    std::lock_guard lock{m_mutex};
    return alloc_unlocked(count);
  }

  uint32_t alloc_image(uint32_t count = 1) noexcept {
    std::lock_guard lock{m_mutex};
    return alloc_unlocked(count);
  }

  void free_buffer(uint32_t index, uint32_t count = 1) noexcept {
    std::lock_guard lock{m_mutex};
    assert(index != INVALID_INDEX);
    m_indexPool.free(index, count);
  }

  void free_image(uint32_t index, uint32_t count = 1) noexcept {
    std::lock_guard lock{m_mutex};
    assert(index != INVALID_INDEX);
    m_indexPool.free(index, count);
  }

  uint64_t buffer_stride() const noexcept { return m_stride; }

  uint64_t image_stride() const noexcept { return m_stride; }

  uint64_t reserved_range_offset() const noexcept {
    std::lock_guard lock{m_mutex};
    return descriptor_region_size_unlocked();
  }

  uint64_t reserved_range_size() const noexcept {
    std::lock_guard lock{m_mutex};
    return m_bufferSize - descriptor_region_size_unlocked();
  }

  uint64_t descriptor_region_size() const noexcept {
    std::lock_guard lock{m_mutex};
    return descriptor_region_size_unlocked();
  }

  uint32_t descriptor_count() const noexcept {
    std::lock_guard lock{m_mutex};
    return m_descriptorCount;
  }

  void grow(uint64_t newBufferSize,
            const vulkan::DescriptorHeapProperties &properties) noexcept {
    std::lock_guard lock{m_mutex};
    assert(unified_stride(properties) == m_stride);
    assert(properties.minResourceHeapReservedRange == m_minReservedRange);
    assert(properties.maxResourceHeapSize == m_maxBufferSize);
    assert(newBufferSize >= m_bufferSize);
    assert(newBufferSize <= m_maxBufferSize);
    const uint32_t newDescriptorCount = slot_count(newBufferSize, properties);
    assert(newDescriptorCount >= m_descriptorCount);
    m_indexPool.resize(newDescriptorCount);
    m_bufferSize = newBufferSize;
    m_descriptorCount = newDescriptorCount;
  }

private:
  using index_pool = BitmapIndexPool<strobe::rhi::allocator_ref>;

  uint32_t alloc_unlocked(uint32_t count) noexcept {
    const std::size_t index = m_indexPool.alloc(count);
    if (index == index_pool::INVALID_INDEX) {
      return INVALID_INDEX;
    }
    assert(index < INVALID_INDEX);
    return static_cast<uint32_t>(index);
  }

  uint64_t descriptor_region_size_unlocked() const noexcept {
    return static_cast<uint64_t>(m_descriptorCount) * m_stride;
  }

  const uint64_t m_stride;
  const uint64_t m_minReservedRange;
  const uint64_t m_maxBufferSize;

  uint64_t m_bufferSize;
  uint32_t m_descriptorCount;
  index_pool m_indexPool;
  mutable std::mutex m_mutex;
};

} // namespace strobe::rhi
