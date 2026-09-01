#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <limits>
#include <mutex>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

class TimelineBarrier {
public:
  explicit TimelineBarrier(Context context, span<Timeline> timelines,
                           strobe::rhi::allocator_ref alloc) noexcept
      : m_context(std::move(context)), m_timelines(timelines, alloc), m_rr(0),
        m_values(m_timelines.size() + 1, alloc),
        m_semaphores(m_timelines.size() + 1, alloc) {
    assert(!timelines.empty());
    for (uint32_t i = 0; i < m_timelines.size(); ++i) {
      assert(m_timelines[i]);
      m_values[i] = m_timelines[i].serial();
      m_semaphores[i] = m_timelines[i].timelineSemaphore().handle;
    }
    m_semaphores.back() =
        vulkan::create_timeline_semaphore(m_context.ctx(), {}).handle;
  }
  ~TimelineBarrier() noexcept {
    vulkan::destroy_timeline_semaphore(m_context.ctx(),
                                       {.handle = m_semaphores.back()});
  }

  void notify() {
    std::lock_guard lock{m_mutex};
    const uint64_t value = m_wakeValue + 1;
    vulkan::signal_timeline_semaphore(m_context.ctx(),
                                      {.handle = m_semaphores.back()}, value);
    m_wakeValue = value;
  }

  std::pair<uint32_t, Timepoint>
  wait_any(uint64_t timeout = std::numeric_limits<uint64_t>::max()) {
    vulkan::Context *ctx = m_context.ctx();
    assert(ctx);

    uint64_t wake;
    {
      std::lock_guard lck{m_mutex};
      wake = m_wakeValue + 1;
    }
    m_values.back() = wake;

    // for (uint32_t i = 0; i < m_timelines.size(); ++i) {
    //   m_timelines[i].notify(m_values[i]);
    // }

    VkSemaphoreWaitInfo waitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .pNext = nullptr,
        .flags = VK_SEMAPHORE_WAIT_ANY_BIT,
        .semaphoreCount = static_cast<uint32_t>(m_semaphores.size()),
        .pSemaphores = m_semaphores.data(),
        .pValues = m_values.data(),
    };

    VkResult result;
    {
      ZoneScopedN("vkWaitSemaphores");
      result = vkWaitSemaphores(ctx->device(), &waitInfo, timeout);
    }
    if (result == VK_TIMEOUT) {
      return {std::numeric_limits<uint32_t>::max(), Timepoint{}};
    }
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to wait for timeline semaphore");
    }

    const uint32_t end = m_rr;
    do {
      Timeline timeline = m_timelines[m_rr];
      if (timeline.now().m_serial > m_values[m_rr]) {
        const uint64_t value = vulkan::get_timeline_semaphore_value(
            ctx, vulkan::TimelineSemaphore{.handle = m_semaphores[m_rr]});
        if (value >= m_values[m_rr]) {
          const uint32_t index = m_rr;
          m_values[index] = value + 1;
          ++m_rr;
          if (m_rr >= m_timelines.size()) {
            m_rr -= m_timelines.size();
          }
          return {index, timeline.from_serial(value)};
        }
      }
      ++m_rr;
      if (m_rr >= m_timelines.size()) {
        m_rr -= m_timelines.size();
      }
    } while (m_rr != end);

    // No queue timeline satisfied its watched value, therefore the wake
    // semaphore did.
#ifndef NDEBUG
    {
      std::lock_guard lock{m_mutex};
      assert(m_wakeValue >= wake);
    }
#endif
    return {std::numeric_limits<uint32_t>::max(), {}};
  }

private:
  Context m_context;
  Vector<Timeline, strobe::rhi::allocator_ref> m_timelines;
  uint32_t m_rr;
  Vector<uint64_t, strobe::rhi::allocator_ref> m_values;
  Vector<VkSemaphore, strobe::rhi::allocator_ref> m_semaphores;
  std::mutex m_mutex;
  uint64_t m_wakeValue{0};
};

} // namespace strobe::rhi
