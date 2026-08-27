#pragma once

#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/buffer.hpp"

namespace strobe::rhi {

struct ScratchBufferImpl {
public:
  explicit ScratchBufferImpl(MemoryPool memoryPool,
                             buf::handle_allocators *bufAllocators) noexcept
      : m_memoryPool(std::move(memoryPool)), m_bufAllocators(bufAllocators),
        m_scratchBuffer{} {}

  Buffer scratch() { return m_scratchBuffer; }

  void require(VkDeviceSize size) {
    if (size < m_scratchBuffer.size()) {
      // allocate new scratch buffer
      m_scratchBuffer = buf::create_buffer(
          m_memoryPool,
          {
              .size = size,
              .bufferUsage =
                  BufferUsage::storage | BufferUsage::shader_device_address,
              .memoryUsage = MemoryUsage::automatic,
          },
          {}, m_bufAllocators);
    }
  }

private:
  MemoryPool m_memoryPool;
  buf::handle_allocators *m_bufAllocators;
  Buffer m_scratchBuffer;
};

} // namespace strobe::rhi
