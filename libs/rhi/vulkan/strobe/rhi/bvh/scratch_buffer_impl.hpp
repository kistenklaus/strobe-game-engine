#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include <algorithm>

namespace strobe::rhi {

struct ScratchBufferImpl {
public:
  explicit ScratchBufferImpl(MemoryPool memoryPool,
                             buf::handle_allocators *bufAllocators) noexcept
      : m_memoryPool(std::move(memoryPool)), m_bufAllocators(bufAllocators),
        m_scratchBuffer{} {}

  Buffer scratch() { return m_scratchBuffer; }

  VkDeviceAddress address() noexcept { return m_address; }

  void require(VkDeviceSize size) {
    size = std::max(size, VkDeviceSize{1});
    if (size > m_scratchBuffer.size()) {
      // allocate new scratch buffer
      uint64_t alignment = m_memoryPool.context()
                               .ctx()
                               ->deviceInfo()
                               .properties.accelerationStructure
                               .minAccelerationStructureScratchOffsetAlignment;
      m_scratchBuffer = buf::create_buffer(
          m_memoryPool,
          {
              .size = size + alignment - 1,
              .bufferUsage =
                  BufferUsage::storage | BufferUsage::shader_device_address,
              .memoryUsage = MemoryUsage::automatic,
          },
          {}, m_bufAllocators);
      auto *buf_impl = object_handle_ptr<BufferImpl>(m_scratchBuffer);
      buf_impl->commit();
      m_address = memory::align_up(buf_impl->address, alignment);
    }
  }

private:
  MemoryPool m_memoryPool;
  buf::handle_allocators *m_bufAllocators;
  Buffer m_scratchBuffer;
  VkDeviceAddress m_address;
};

} // namespace strobe::rhi
