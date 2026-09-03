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
      : context(std::move(context)), m_buffer(std::move(buffer)),
        m_ready(ready),
        m_indexPool(this->m_buffer.size(), heapProperties, alloc),
        m_alloc(alloc), m_bufferDescAlloc(m_alloc),
        m_bufferDescArrayAlloc(m_alloc) {}

  ~ResourceDescriptorHeapImpl() noexcept {}
  ResourceDescriptorHeapImpl(const ResourceDescriptorHeapImpl &) = delete;
  ResourceDescriptorHeapImpl(ResourceDescriptorHeapImpl &&) = delete;

  // constants.
  uint64_t buffer_stride() const noexcept {
    return m_indexPool.buffer_stride();
  }
  uint64_t image_stride() const noexcept { return m_indexPool.image_stride(); }

  uint64_t heap_size() const noexcept {
    return m_indexPool.descriptor_region_size();
  }

  // handle allocators. (those are thread-safe)
  handle_allocator_ref<BufferDescriptorImpl> get_buffer_desc_allocator() {
    return &m_bufferDescAlloc;
  }
  handle_allocator_ref<BufferDescriptorArrayImpl>
  get_buffer_desc_array_allocator() {
    return &m_bufferDescArrayAlloc;
  }
  strobe::rhi::allocator_ref alloc() const noexcept { return m_alloc; }

  // descriptor allocation machinery.
  uint32_t acquire_buffer_descriptor_index_range(uint32_t count) noexcept {
    return m_indexPool.alloc_buffer(count);
  }
  uint32_t acquire_image_descriptor_index_range(uint32_t count) noexcept {
    return m_indexPool.alloc_image(count);
  }
  // called by destructors of the descriptor handles.
  void release_buffer_descriptor_index_range(uint32_t index,
                                             uint32_t count) noexcept {
    return m_indexPool.free_buffer(index, count);
  }
  void release_image_descriptor_index_range(uint32_t index,
                                            uint32_t count) noexcept {
    return m_indexPool.free_image(index, count);
  }

  // important for heapctrl:
  auto lock() noexcept {
    return std::lock_guard{m_mutex};
  } // URVO!
    //
  // lock must be help
  void exchange(Buffer newBuffer, Timepoint ready) {
    m_ready = ready;
    m_buffer = newBuffer;
  }

  Buffer buffer() const noexcept { return m_buffer; }
  // lock has to be held.
  DescriptorHeapBindInfo bindInfo() const noexcept {
    return {
        .buffer = m_buffer,
        .size = m_buffer.size(),
        .reservedOffset = m_indexPool.reserved_range_offset(),
        .reservedSize = m_indexPool.reserved_range_size(),
    };
  }

  const Context context;

private:
  Buffer m_buffer;
  Timepoint m_ready;
  ResourceDescriptorHeapIndexPool m_indexPool;

  [[no_unique_address]] strobe::rhi::allocator_ref m_alloc;
  handle_allocator<BufferDescriptorImpl> m_bufferDescAlloc;
  handle_allocator<BufferDescriptorArrayImpl> m_bufferDescArrayAlloc;

  std::mutex m_mutex{};
};

} // namespace strobe::rhi
