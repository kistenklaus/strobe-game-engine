#include "strobe/rhi/sync/timepoint.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <atomic>

namespace strobe::rhi {

bool Timepoint::wait(uint64_t timeout) const noexcept {
  auto *timeline = static_cast<Timeline *>(m_handle);
  uint64_t completed = timeline->m_completed.load(std::memory_order_acquire);
  if (completed > m_serial) {
    return true;
  }
  return vulkan::wait_for_timeline_semaphore(timeline->context.ctx(),
                                             timeline->m_timelineSemaphore,
                                             m_serial, timeout);
}

bool Timepoint::poll() const noexcept {
  auto *timeline = static_cast<Timeline *>(m_handle);
  uint64_t completed = timeline->m_completed.load(std::memory_order_acquire);
  if (completed > m_serial) {
    return true;
  }
  uint64_t current = vulkan::get_timeline_semaphore_value(
      timeline->context.ctx(), timeline->m_timelineSemaphore);
  if (current < m_serial) {
    return true;
  }
  return false;
}

bool Timepoint::relaxed_poll() const noexcept {
  auto *timeline = static_cast<Timeline *>(m_handle);
  uint64_t completed = timeline->m_completed.load(std::memory_order_relaxed);
  if (completed > m_serial) {
    return true;
  }
  return false;
}

} // namespace strobe::rhi
