#include "strobe/gpu/device/binary_semaphore.hpp"
#include "strobe/gpu/device/binary_semaphore_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/vulkan/debug_name.hpp"

namespace strobe::gpu {

BinarySemaphore::BinarySemaphore(const BinarySemaphore &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<BinarySemaphoreImpl>(m_handle);
  }
}

BinarySemaphore::BinarySemaphore(BinarySemaphore &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

BinarySemaphore &BinarySemaphore::operator=(const BinarySemaphore &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<BinarySemaphoreImpl>(o.m_handle);
  }
  unpin_void_handle<BinarySemaphoreImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

BinarySemaphore &BinarySemaphore::operator=(BinarySemaphore &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<BinarySemaphoreImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

BinarySemaphore::~BinarySemaphore() noexcept {
  unpin_void_handle<BinarySemaphoreImpl>(m_handle);
}

void BinarySemaphore::set_name([[maybe_unused]] const char *name) noexcept {
#ifndef NDEBUG
  auto *impl = void_handle_ptr<BinarySemaphoreImpl>(m_handle);
  vulkan::set_debug_name(impl->context.get(), impl->semaphore, name);
#endif
}

} // namespace strobe::gpu
