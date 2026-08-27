#include "strobe/rhi/objects/timeline_semaphore.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/timeline_semaphore_impl.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"

namespace strobe::rhi {

TimelineSemaphore::TimelineSemaphore(const TimelineSemaphore &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<TimelineSemaphoreImpl>(m_handle);
  }
}

TimelineSemaphore::TimelineSemaphore(TimelineSemaphore &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

TimelineSemaphore &
TimelineSemaphore::operator=(const TimelineSemaphore &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<TimelineSemaphoreImpl>(o.m_handle);
  }
  unpin_void_handle<TimelineSemaphoreImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

TimelineSemaphore &
TimelineSemaphore::operator=(TimelineSemaphore &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<TimelineSemaphoreImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

TimelineSemaphore::~TimelineSemaphore() noexcept {
  unpin_void_handle<TimelineSemaphore>(m_handle);
}
uint64_t TimelineSemaphore::count() const {
  ZoneScopedN("TimelineSemaphore::count");
  auto *impl = void_handle_ptr<TimelineSemaphoreImpl>(m_handle);
  return vulkan::get_timeline_semaphore_value(impl->context.ctx(),
                                              impl->semaphore);
}

void TimelineSemaphore::signal(uint64_t value) {
  ZoneScopedN("TimelineSemaphore::signal");
  auto *impl = void_handle_ptr<TimelineSemaphoreImpl>(m_handle);
  vulkan::signal_timeline_semaphore(impl->context.ctx(), impl->semaphore,
                                    value);
}

bool TimelineSemaphore::wait(uint64_t value, uint64_t timeout) {
  ZoneScopedN("TimelineSemaphore::wait");
  auto *impl = void_handle_ptr<TimelineSemaphoreImpl>(m_handle);
  return vulkan::wait_for_timeline_semaphore(impl->context.ctx(),
                                             impl->semaphore, value, timeout);
}

} // namespace strobe::rhi
