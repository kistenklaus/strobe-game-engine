#include "strobe/rhi/sync/timepoint.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/timeline_impl.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <atomic>

namespace strobe::rhi {

Timepoint::Timepoint(void *handle, uint64_t serial) noexcept
    : m_handle(handle), m_serial(serial) {
  pin_void_handle<TimelineImpl>(m_handle);
}

Timepoint::Timepoint(const Timepoint &o) noexcept
    : m_handle(o.m_handle), m_serial(o.m_serial) {
  if (m_handle != nullptr) {
    pin_void_handle<TimelineImpl>(m_handle);
  }
}

Timepoint::Timepoint(Timepoint &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)),
      m_serial(std::exchange(o.m_serial, 0)) {}

Timepoint &Timepoint::operator=(const Timepoint &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<TimelineImpl>(o.m_handle);
  }
  unpin_void_handle<TimelineImpl>(m_handle);
  m_handle = o.m_handle;
  m_serial = o.m_serial;
  return *this;
}

Timepoint &Timepoint::operator=(Timepoint &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<TimelineImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  m_serial = std::exchange(o.m_serial, 0);
  return *this;
}

Timepoint::~Timepoint() noexcept { unpin_void_handle<TimelineImpl>(m_handle); }

bool Timepoint::wait(uint64_t timeout) const noexcept {
  auto *timeline = void_handle_ptr<TimelineImpl>(m_handle);
  uint64_t completed = timeline->m_completed.load(std::memory_order_acquire);
  if (completed >= m_serial) {
    return true;
  }
  return vulkan::wait_for_timeline_semaphore(timeline->context.ctx(),
                                             timeline->m_timelineSemaphore,
                                             m_serial, timeout);
}

bool Timepoint::poll() const noexcept {
  auto *timeline = void_handle_ptr<TimelineImpl>(m_handle);
  uint64_t completed = timeline->m_completed.load(std::memory_order_acquire);
  if (completed >= m_serial) {
    return true;
  }
  uint64_t current = vulkan::get_timeline_semaphore_value(
      timeline->context.ctx(), timeline->m_timelineSemaphore);
  return current >= m_serial;
}

bool Timepoint::relaxed_poll() const noexcept {
  auto *timeline = void_handle_ptr<TimelineImpl>(m_handle);
  uint64_t completed = timeline->m_completed.load(std::memory_order_relaxed);
  if (completed >= m_serial) {
    return true;
  }
  return false;
}

} // namespace strobe::rhi
