#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/sync/timeline.hpp"
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
    assert(!m_timelines.empty());

    for (uint32_t i = 0; i < m_timelines.size(); ++i) {
      assert(m_timelines[i]);

      m_values[i] = m_timelines[i].serial();
      m_semaphores[i] = m_timelines[i].timelineSemaphore().handle;
    }

    m_semaphores.back() =
        vulkan::create_timeline_semaphore(m_context.ctx(), {}).handle;
    m_values.back() = 1;
  }

  ~TimelineBarrier() noexcept {
    vulkan::destroy_timeline_semaphore(
        m_context.ctx(),
        vulkan::TimelineSemaphore{.handle = m_semaphores.back()});
  }

  void notify() {
    std::lock_guard lock{m_wakeMutex};

    const uint64_t value = ++m_wakeValue;

    vulkan::signal_timeline_semaphore(
        m_context.ctx(),
        vulkan::TimelineSemaphore{.handle = m_semaphores.back()}, value);
  }

  std::pair<uint32_t, Timepoint>
  wait_any(uint64_t timeout = std::numeric_limits<uint64_t>::max()) {
    vulkan::Context *ctx = m_context.ctx();
    assert(ctx);

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
      return {
          std::numeric_limits<uint32_t>::max(),
          Timepoint{},
      };
    }

    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to wait for timeline semaphore");
    }

    // Prefer returning queue progress if both a queue timeline and the wake
    // semaphore became ready. The wake remains unconsumed and consequently
    // makes the next wait return immediately.
    const uint32_t end = m_rr;

    do {
      Timeline timeline = m_timelines[m_rr];
      const uint64_t target = m_values[m_rr];

      // Avoid querying values which have not even been closed/submitted yet.
      if (timeline.serial() > target) {
        const uint64_t value = vulkan::get_timeline_semaphore_value(
            ctx,
            vulkan::TimelineSemaphore{.handle = m_semaphores[m_rr]});

        if (value >= target) {
          const uint32_t index = m_rr;

          // Wait for the next value from this timeline.
          m_values[index] = value + 1;

          advance_round_robin();

          return {
              index,
              timeline.from_serial(value),
          };
        }
      }

      advance_round_robin();
    } while (m_rr != end);

    // No queue timeline satisfied its target, so the wake semaphore must have
    // satisfied the VK_SEMAPHORE_WAIT_ANY_BIT wait.
    const uint64_t wakeTarget = m_values.back();
    const uint64_t wakeValue = vulkan::get_timeline_semaphore_value(
        ctx, vulkan::TimelineSemaphore{.handle = m_semaphores.back()});

    assert(wakeValue >= wakeTarget);

    // Coalesce every notification which has already been signaled. If another
    // notification races this query, the next target will already be
    // satisfied and wait_any() will immediately wake again.
    m_values.back() = wakeValue + 1;

    return {
        std::numeric_limits<uint32_t>::max(),
        Timepoint{},
    };
  }

private:
  void advance_round_robin() noexcept {
    ++m_rr;
    if (m_rr == m_timelines.size()) {
      m_rr = 0;
    }
  }

  Context m_context;
  Vector<Timeline, strobe::rhi::allocator_ref> m_timelines;
  uint32_t m_rr;

  // One target per queue timeline, plus the wake semaphore target.
  // Only the wait/GC thread accesses this vector.
  Vector<uint64_t, strobe::rhi::allocator_ref> m_values;
  Vector<VkSemaphore, strobe::rhi::allocator_ref> m_semaphores;

  // Serializes multiple concurrent producers so wake values are signaled in
  // strictly increasing order.
  std::mutex m_wakeMutex;
  uint64_t m_wakeValue{0};
};

} // namespace strobe::rhi
