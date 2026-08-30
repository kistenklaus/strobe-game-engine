#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/sync/timepoint.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <atomic>
#include <cassert>
#include <mutex>

namespace strobe::rhi {

class Timeline {
  friend class TimelineBarrier;
  friend class Timepoint;

public:
  explicit Timeline(Context context)
      : context(std::move(context)),
        m_timelineSemaphore(
            vulkan::create_timeline_semaphore(this->context.ctx())) {}
  virtual ~Timeline() noexcept {
    vulkan::destroy_timeline_semaphore(context.ctx(), m_timelineSemaphore);
  }

  Timeline(const Timeline &) = delete;
  Timeline(Timeline &&) = delete;
  Timeline &operator=(const Timeline &) = delete;
  Timeline &operator=(Timeline &&) = delete;

  Timepoint now() noexcept {
    return Timepoint{static_cast<void *>(this),
                     m_serial.load(std::memory_order_acquire)};
  }

  bool contains(Timepoint timepoint) const noexcept {
    return timepoint.m_handle == this;
  }

  // returns a timepoint to the beginning of time.
  Timepoint epoch() noexcept { return Timepoint{this, 0}; }

  // ensures that the timepoints timeline as commited this timepoint
  static void notify(Timepoint timepoint) noexcept {
    if (timepoint.relaxed_poll()) {
      return;
    }
    auto *timeline = static_cast<Timeline *>(timepoint.m_handle);
    std::lock_guard lck{timeline->m_mutex};
    uint64_t serial = timeline->m_serial.load(std::memory_order_relaxed);
    if (timepoint.m_serial < serial) {
      return;
    }
    assert(timepoint.m_serial == serial);
    timeline->commit(timepoint);
    timeline->m_serial.store(serial + 1, std::memory_order_release);
  }

  void complete(Timepoint timepoint) noexcept {
    m_completed.store(timepoint.m_serial, std::memory_order_release);
  }

protected:
  static vulkan::TimelineSemaphore
  get_timepoint_semaphore(Timepoint timepoint) {
    return static_cast<Timeline *>(timepoint.m_handle)->m_timelineSemaphore;
  }
  static uint64_t get_timepoint_serial(Timepoint timepoint) {
    return timepoint.m_serial;
  }

  /// advances timeline and returns old timepoint.
  Timepoint advance() {
    return Timepoint{this, m_serial.fetch_add(1, std::memory_order_release)};
  }
  struct TimelineSignal {
    vulkan::TimelineSemaphore semaphore;
    uint64_t value;
  };

  // internally synchronized
  // Is not allowed to call Timeline::notify in it's implementation
  virtual void commit(Timepoint timepoint) noexcept = 0;

  const Context context;

  const vulkan::TimelineSemaphore m_timelineSemaphore;

private:
  mutable std::mutex m_mutex;
  mutable std::atomic<uint64_t> m_serial{0};
  mutable std::atomic<uint64_t> m_completed{0};
};

} // namespace strobe::rhi
