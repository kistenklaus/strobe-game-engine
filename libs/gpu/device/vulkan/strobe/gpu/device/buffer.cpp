#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/buffer_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/memory_allocation_handle_alloc.hpp"
#include "strobe/gpu/device/memory_allocation_impl.hpp"
#include "strobe/gpu/device/memory_pool_impl.hpp"
#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/debug_name.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

using handle_alloc = buffer_handle_alloc_ref;

void unpin_buffer(void *h) noexcept {
  if (h == nullptr) {
    return;
  }
  using control_block = handle_control_block<BufferImpl, handle_alloc>;
  using allocator_traits = AllocatorTraits<handle_alloc>;

  auto *block = static_cast<control_block *>(h);

  if (block->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }
  MemoryAllocation keepAlive = block->value.allocation;
  handle_alloc alloc = std::move(block->alloc);
  std::destroy_at(block);
  // alloc, still has to be valid after destroy!, but destroy might destroy the
  // underlying pool!
  allocator_traits::template deallocate<control_block>(alloc, block);
}

Buffer::Buffer(const Buffer &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<BufferImpl, handle_alloc>(m_handle);
  }
}

Buffer::Buffer(Buffer &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Buffer &Buffer::operator=(const Buffer &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<BufferImpl>(o.m_handle);
  }
  unpin_buffer(m_handle);
  // unpin_void_handle<BufferImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Buffer &Buffer::operator=(Buffer &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_buffer(m_handle);
  // unpin_void_handle<BufferImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Buffer::~Buffer() noexcept {
  unpin_buffer(m_handle);
  // unpin_void_handle<BufferImpl>(m_handle);
}

uint64_t Buffer::size() const noexcept {
  if (m_handle == nullptr) {
    return 0;
  }
  auto *impl = void_handle_ptr<BufferImpl, handle_alloc>(m_handle);
  return impl->size;
}

void Buffer::commit() const {
  assert(m_handle);
  auto *impl = void_handle_ptr<BufferImpl, handle_alloc>(m_handle);
  assert(impl->allocation);
  auto *mem_impl = void_handle_ptr<MemoryAllocationImpl,
                                   memory_allocation_handle_allocator_ref>(
      impl->allocation.m_handle);
  assert(mem_impl);
  if (!mem_impl->binding) {
    assert(mem_impl->pool);
    auto *pool_impl = void_handle_ptr<MemoryPoolImpl>(mem_impl->pool.m_handle);
    mem_impl->binding =
        pool_impl->bind_buffer(impl->buffer, mem_impl->internals);
    impl->address =
        vulkan::get_buffer_device_address(impl->context.get(), impl->buffer);
  }
}
void *Buffer::ptr() const {
  auto *impl = void_handle_ptr<BufferImpl, handle_alloc>(m_handle);
  commit();
  return impl->allocation.map();
}

void Buffer::set_name(const char *name) const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<BufferImpl, handle_alloc>(m_handle);
  vulkan::set_debug_name(impl->context.get(), impl->buffer, name);
}

} // namespace strobe::gpu
