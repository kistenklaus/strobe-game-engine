#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array_impl.hpp"
#include "strobe/rhi/heap/buffer_descriptor_impl.hpp"
#include "strobe/rhi/heap/descriptor_heap_bind_info.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_index_pool.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/vulkan/device_info/device_properties.hpp"

#include <mutex>
#include <tracy/Tracy.hpp>

namespace strobe::rhi {

struct ResourceDescriptorHeapImpl {
public:
  explicit ResourceDescriptorHeapImpl(
      Context context, Buffer buffer, Timepoint ready,
      const vulkan::DescriptorHeapProperties &heapProperties,
      strobe::rhi::allocator_ref alloc) noexcept
      : context(std::move(context)),
        layout(buffer.size(), heapProperties, alloc), alloc(alloc),
        bufferDescAlloc(alloc), bufferDescArrayAlloc(alloc),
        m_buffer(std::move(buffer)), m_ready(ready),
        m_reservedOffset(layout.reserved_range_offset()),
        m_reservedSize(layout.reserved_range_size()) {}

  ~ResourceDescriptorHeapImpl() noexcept {}
  ResourceDescriptorHeapImpl(const ResourceDescriptorHeapImpl &) = delete;
  ResourceDescriptorHeapImpl(ResourceDescriptorHeapImpl &&) = delete;

  void exchange(Buffer newBuffer, Timepoint ready) {
    std::lock_guard lck{m_mutex};
    layout.grow(newBuffer.size(),
                context.ctx()->deviceInfo().properties.descriptorHeap);
    m_ready = ready;
    m_buffer = newBuffer;
    m_reservedOffset = layout.reserved_range_offset();
    m_reservedSize = layout.reserved_range_size();
  }

  Buffer buffer() const noexcept {
    std::lock_guard lck{m_mutex};
    return m_buffer;
  }

  DescriptorHeapBindInfo bindInfo() const noexcept {
    std::lock_guard lck{m_mutex};
    return {
        .buffer = m_buffer,
        .size = m_buffer.size(),
        .reservedOffset = m_reservedOffset,
        .reservedSize = m_reservedSize,
        .ready = m_ready,
    };
  }

  const Context context;
  ResourceDescriptorHeapIndexPool layout; // <- internally synchronized.

  [[no_unique_address]] strobe::rhi::allocator_ref alloc;
  handle_allocator<BufferDescriptorImpl> bufferDescAlloc;
  handle_allocator<BufferDescriptorArrayImpl> bufferDescArrayAlloc;

private:
  // Synchronized by mutex.
  // NOTE: We will have to see how much
  // contension we get on this lock for
  // heavy parallel recording.
  Buffer m_buffer;
  Timepoint m_ready;
  uint64_t m_reservedOffset;
  uint64_t m_reservedSize;
#ifdef STROBE_RHI_TRACE_LOCKS
  mutable TracyLockableN(std::mutex, m_mutex, "rheap-mutex");
#else
  mutable std::mutex m_mutex{};
#endif
};

} // namespace strobe::rhi
