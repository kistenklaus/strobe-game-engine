#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <atomic>
#include <cassert>
#include <fmt/ostream.h>
#include <mutex>
namespace strobe::rhi {

struct TimelineImpl {
public:
  explicit TimelineImpl(Context context, vulkan::TimelineSemaphore timeline)
      : context(std::move(context)), m_timelineSemaphore(timeline) {}
  ~TimelineImpl() noexcept {
    vulkan::destroy_timeline_semaphore(context.ctx(), m_timelineSemaphore);
  }

  TimelineImpl(const TimelineImpl &) = delete;
  TimelineImpl(TimelineImpl &&) = delete;
  TimelineImpl &operator=(const TimelineImpl &) = delete;
  TimelineImpl &operator=(TimelineImpl &&) = delete;

  static vulkan::TimelineSemaphore
  get_timepoint_semaphore(Timepoint timepoint) {
    return void_handle_ptr<TimelineImpl>(timepoint.m_handle)
        ->m_timelineSemaphore;
  }
  static uint64_t get_timepoint_serial(Timepoint timepoint) {
    return timepoint.m_serial;
  }

  const Context context;
  const vulkan::TimelineSemaphore m_timelineSemaphore;
  std::mutex m_mutex{};
  std::atomic<uint64_t> m_serial{1};
  std::atomic<uint64_t> m_completed{0};

  void *m_pUserData = nullptr;
  void (*m_commit)(void *, Timepoint) = nullptr;
};

} // namespace strobe::rhi
