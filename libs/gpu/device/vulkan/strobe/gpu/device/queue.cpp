#include "strobe/gpu/device/queue.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/queue_impl.hpp"

namespace strobe::gpu {

Queue::Queue(const Queue &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<QueueImpl>(m_handle);
  }
}

Queue::Queue(Queue &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Queue &Queue::operator=(const Queue &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<QueueImpl>(o.m_handle);
  }
  unpin_void_handle<QueueImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}
Queue &Queue::operator=(Queue &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<QueueImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Queue::~Queue() noexcept { unpin_void_handle<QueueImpl>(m_handle); }

} // namespace strobe::gpu
