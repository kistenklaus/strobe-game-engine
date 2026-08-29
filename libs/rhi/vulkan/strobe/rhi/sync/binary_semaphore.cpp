#include "strobe/rhi/objects/binary_semaphore.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/binary_semaphore_impl.hpp"
#ifndef NDEBUG
#include "strobe/rhi/vulkan/debug_name.hpp"
#endif

namespace strobe::rhi {

BinarySemaphore::BinarySemaphore(const BinarySemaphore &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<BinarySemaphoreImpl>(m_handle);
  }
}

BinarySemaphore::BinarySemaphore(BinarySemaphore &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

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
  vulkan::set_debug_name(impl->context.ctx(), impl->semaphore, name);
#endif
}

} // namespace strobe::rhi
