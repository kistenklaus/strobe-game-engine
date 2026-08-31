#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/timeline_impl.hpp"
#include <thread>

namespace strobe::rhi {

Timeline::Timeline(const Timeline &o) noexcept : Object(o.m_handle) {
  if (m_handle) {
    pin_void_handle<TimelineImpl>(m_handle);
  }
}

Timeline::Timeline(Timeline &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

Timeline &Timeline::operator=(const Timeline &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<TimelineImpl>(o.m_handle);
  }
  unpin_void_handle<TimelineImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Timeline &Timeline::operator=(Timeline &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<TimelineImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Timeline::~Timeline() noexcept { unpin_void_handle<TimelineImpl>(m_handle); }

void Timeline::notify(Timepoint timepoint) noexcept {
  if (timepoint.relaxed_poll()) {
    return;
  }
  auto *timeline = void_handle_ptr<TimelineImpl>(timepoint.m_handle);
  {
    std::lock_guard lck{
        timeline->m_mutex}; // <- somehow this ends up beeing a deadlock

    uint64_t serial = timeline->m_serial.load(std::memory_order_relaxed);
    if (timepoint.m_serial < serial) {
      return;
    }
    assert(timepoint.m_serial == serial);
    if (timeline->m_commit) {
      timeline->m_commit(timeline->m_pUserData, timepoint);
    }
  }
}

void Timeline::notify(uint64_t serial) noexcept {
  assert(m_handle);
  notify(Timepoint{m_handle, serial});
}

Timepoint Timeline::now() noexcept {
  auto *impl = void_handle_ptr<TimelineImpl>(m_handle);
  return Timepoint{m_handle, impl->m_serial.load(std::memory_order_acquire)};
}

bool Timeline::contains(Timepoint timepoint) const noexcept {
  return timepoint.m_handle == m_handle;
}
Timepoint Timeline::epoch() const noexcept { return Timepoint{m_handle, 0}; }

void Timeline::complete(Timepoint timepoint) noexcept {
  auto *impl = void_handle_ptr<TimelineImpl>(m_handle);
  return impl->m_completed.store(timepoint.m_serial, std::memory_order_release);
}

Timepoint Timeline::advance() noexcept {
  auto *impl = void_handle_ptr<TimelineImpl>(m_handle);
  return Timepoint{m_handle,
                   impl->m_serial.fetch_add(1, std::memory_order_release)};
}

uint64_t Timeline::serial() const noexcept {
  auto *impl = void_handle_ptr<TimelineImpl>(m_handle);
  return impl->m_serial.load(std::memory_order_acquire);
}

uint64_t Timeline::completed_serial() const noexcept {
  auto *impl = void_handle_ptr<TimelineImpl>(m_handle);
  return impl->m_completed;
}

vulkan::TimelineSemaphore Timeline::timelineSemaphore() const noexcept {
  auto *impl = void_handle_ptr<TimelineImpl>(m_handle);
  return impl->m_timelineSemaphore;
}

void Timeline::set_commit_callback(void *pUserData,
                                   void (*commit)(void *, Timepoint)) noexcept {
  auto *impl = void_handle_ptr<TimelineImpl>(m_handle);

  std::lock_guard lck{impl->m_mutex};
  impl->m_pUserData = pUserData;
  impl->m_commit = commit;
}
void Timeline::clear_callback() noexcept {
  auto *impl = void_handle_ptr<TimelineImpl>(m_handle);
  std::lock_guard lck{impl->m_mutex};
  impl->m_pUserData = nullptr;
  impl->m_commit = nullptr;
}

Timepoint Timeline::from_serial(uint64_t serial) noexcept {
  return Timepoint{m_handle, serial};
}

} // namespace strobe::rhi
