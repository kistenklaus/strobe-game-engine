#pragma once

#include "strobe/core/memory/align.hpp"
#include "strobe/rhi/command/staging_buffer.hpp"
#include "strobe/rhi/memory/buffer_binding.hpp"

namespace strobe::rhi {

struct StagingBumpAllocator {
public:
  explicit StagingBumpAllocator(StagingBuffer *buffer) noexcept : m_buffer(buffer) {}
  StagingBumpAllocator() noexcept : m_buffer(nullptr) {}

  BufferBinding try_alloc(VkDeviceSize size, uint16_t alignment = 1) {
    if (m_buffer == nullptr) {
      return {};
    }
    assert(alignment != 0);
    assert(std::has_single_bit(alignment));

    VkDeviceSize offset = strobe::memory::align_up(m_top, alignment);
    if (offset > m_buffer->size || size > m_buffer->size - offset) {
      return {};
    }

    BufferBinding binding{
        .buffer = m_buffer->buffer,
        .offset = offset,
        .size = size,
        .mapped = static_cast<std::byte *>(m_buffer->mapped) + offset,
    };
    m_top = offset + size;
    return binding;
  }

  StagingBuffer *buffer() const noexcept { return m_buffer; }

  VkDeviceSize remaining() const noexcept {
    if (m_buffer == nullptr) {
      return 0;
    }
    return m_buffer->size - m_top;
  }

private:
  VkDeviceSize m_top = 0;
  StagingBuffer *m_buffer;
};

} // namespace strobe::rhi
