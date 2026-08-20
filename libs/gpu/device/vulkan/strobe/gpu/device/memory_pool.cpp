#include "strobe/gpu/device/memory_pool.hpp"
#include "strobe/gpu/device/buffer_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/memory_allocation_handle_alloc.hpp"
#include "strobe/gpu/device/memory_pool_impl.hpp"
#include "strobe/gpu/vulkan/buffer.hpp"
#include <stdexcept>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

MemoryPool::MemoryPool(const MemoryPool &o) noexcept : m_handle(o.m_handle) {
  if (m_handle) {
    pin_void_handle<MemoryPoolImpl>(m_handle);
  }
}

MemoryPool::MemoryPool(MemoryPool &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

MemoryPool &MemoryPool::operator=(const MemoryPool &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<MemoryPoolImpl>(o.m_handle);
  }
  unpin_void_handle<MemoryPoolImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

MemoryPool &MemoryPool::operator=(MemoryPool &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<MemoryPoolImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

MemoryPool::~MemoryPool() noexcept {
  unpin_void_handle<MemoryPoolImpl>(m_handle);
}

Buffer MemoryPool::create_buffer(const BufferCreateInfo &info,
                                 const MemoryLifetime &lifetime) {
  ZoneScopedN("MemoryPool::create_buffer");
  auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);

  auto [binding, buffer, internals] = impl->create_buffer(info, lifetime);
  MemoryAllocation memoryAllocation{
      alloc_void_handle<MemoryAllocationImpl,
                        memory_allocation_handle_allocator_ref>(
          impl->get_handle_alloc(), *this, binding, internals)};
  VkDeviceAddress address = 0;
  if (binding) {
    address = vulkan::get_buffer_device_address(impl->context.get(), buffer);
  }
  return Buffer{alloc_void_handle<BufferImpl, buffer_handle_alloc_ref>(
      impl->get_buffer_handle_alloc(), impl->context,
      std::move(memoryAllocation), buffer, info.size, address)};
}

void MemoryPool::commit() {
  ZoneScopedN("MemoryPool::commit");
  [[maybe_unused]] auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);
  impl->commit_all();
}

} // namespace strobe::gpu
