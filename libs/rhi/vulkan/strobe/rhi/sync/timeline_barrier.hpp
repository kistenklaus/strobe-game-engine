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
    const uint64_t requested = ++m_wakeRequested;
    if (!m_waiting) {
      return;
    }
    assert(requested > m_wakeSignaled);
    m_wakeSignaled = requested;
    vulkan::signal_timeline_semaphore(
        m_context.ctx(),
        vulkan::TimelineSemaphore{.handle = m_semaphores.back()}, requested);
  }

  std::pair<uint32_t, Timepoint>
  wait_any(uint64_t timeout = std::numeric_limits<uint64_t>::max()) {
    vulkan::Context *ctx = m_context.ctx();
    assert(ctx);
    {
      std::lock_guard lock{m_wakeMutex};
      if (consume_wake_if_pending()) {
        return {
            std::numeric_limits<uint32_t>::max(),
            Timepoint{},
        };
      }
      m_waiting = true;
    }

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

    {
      std::lock_guard lock{m_wakeMutex};
      m_waiting = false;
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
    const uint32_t end = m_rr;
    do {
      Timeline timeline = m_timelines[m_rr];
      const uint64_t target = m_values[m_rr];

      if (timeline.serial() > target) {
        const uint64_t value = vulkan::get_timeline_semaphore_value(
            ctx, vulkan::TimelineSemaphore{.handle = m_semaphores[m_rr]});
        if (value >= target) {
          const uint32_t index = m_rr;
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
    {
      std::lock_guard lock{m_wakeMutex};
      [[maybe_unused]] const bool consumed = consume_wake_if_pending();
      assert(consumed);
    }

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

  // m_wakeMutex must be held.
  bool consume_wake_if_pending() noexcept {
    if (m_wakeConsumed == m_wakeRequested) {
      return false;
    }

    ++m_wakeConsumed;
    m_values.back() = m_wakeConsumed + 1;
    return true;
  }

  Context m_context;
  Vector<Timeline, strobe::rhi::allocator_ref> m_timelines;
  uint32_t m_rr;

  Vector<uint64_t, strobe::rhi::allocator_ref> m_values;
  Vector<VkSemaphore, strobe::rhi::allocator_ref> m_semaphores;

  std::mutex m_wakeMutex;
  uint64_t m_wakeRequested{0};
  uint64_t m_wakeConsumed{0};
  uint64_t m_wakeSignaled{0};
  bool m_waiting{false};
};

} // namespace strobe::rhi
