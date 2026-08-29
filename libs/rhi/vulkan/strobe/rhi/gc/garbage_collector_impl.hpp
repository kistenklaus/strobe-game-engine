#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/containers/vector_deque.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/sync/timeline_barrier.hpp"
#include "strobe/rhi/sync/timepoint.hpp"
#include <limits>
#include <mutex>
#include <thread>
#include <stop_token>
#include <cassert>
#include <cstdint>

namespace strobe::rhi {

struct GarbageCollectorImpl {
  // If the amount of pending timepoints falls below this
  // threshold the timeline is shall be notified in timely manner.
  // " we call retired but not yet completed timepoints pending.
  static constexpr uint32_t BACKPRESSURE_THRESHOLD = 2;

  explicit GarbageCollectorImpl(Context context,
                                span<Timeline *> timelines)
      : m_context(std::move(context)), m_retireBuffers(timelines.size()),
        m_barrier(m_context, timelines) {
    for (uint32_t i = 0; i < timelines.size(); ++i) {
      m_retireBuffers[i].timeline = timelines[i];
      m_retireBuffers[i].completed = timelines[i]->epoch();
      m_retireBuffers[i].retired = timelines[i]->epoch();
    }
    m_thread = std::jthread(&GarbageCollectorImpl::gc_main, this);
  }

  GarbageCollectorImpl(const GarbageCollectorImpl &) = delete;
  GarbageCollectorImpl(GarbageCollectorImpl &&) = delete;
  GarbageCollectorImpl &operator=(const GarbageCollectorImpl &) = delete;
  GarbageCollectorImpl &operator=(GarbageCollectorImpl &&) = delete;
  ~GarbageCollectorImpl() noexcept = default;

  void retire(Timepoint timepoint, span<const CommandBuffer> cmds) {
    uint32_t timelineIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < m_retireBuffers.size(); ++i) {
      if (m_retireBuffers[i].timeline->contains(timepoint)) {
        timelineIndex = i;
        break;
      }
    }
    assert(timelineIndex != std::numeric_limits<uint32_t>::max());
    auto &buffer = m_retireBuffers[timelineIndex];
    bool wake;
    {
      std::lock_guard lock{buffer.mutex};
      assert(buffer.retired <= timepoint);
      for (const CommandBuffer &cmd : cmds) {
        buffer.retiredCmds.emplace_back(timepoint, cmd);
      }
      buffer.retired = timepoint;
      const uint64_t pending = buffer.retired - buffer.completed;
      wake = pending <= BACKPRESSURE_THRESHOLD;
    }
    if (wake) {
      m_barrier.notify();
    }
  }

  void gc_main(std::stop_token st) {
    std::stop_callback stop_callback(
        st, [barrier = &m_barrier] { barrier->notify(); });

    while (!st.stop_requested()) {
      auto [timelineIndex, timepoint] = m_barrier.wait_any();
      if (st.stop_requested()) {
        break;
      }
      if (timepoint) {
        auto &retireBuffer = m_retireBuffers[timelineIndex];
        retireBuffer.timeline->complete(timepoint);
        collect(retireBuffer, timepoint);
        backpressure(retireBuffer, timepoint);
      } else { // invalid timepoint; just apply backpressure to all timelines
        for (uint32_t i = 0; i < m_retireBuffers.size(); ++i) {
          backpressure(m_retireBuffers[i], m_retireBuffers[i].completed);
        }
      }
    }
  }

private:
  template <typename T> struct Retire {
    Timepoint timepoint;
    T object;
  };

  struct TimelineRetireBuffer {
    Timeline *timeline;
    std::mutex mutex{};
    VectorDeque<Retire<CommandBuffer>> retiredCmds;
    Timepoint completed;
    Timepoint retired;
  };

  void collect(TimelineRetireBuffer &retireBuffer, Timepoint timepoint) {
    collect_retired(retireBuffer.mutex, retireBuffer.retiredCmds, timepoint);
  }

  template <typename T>
  void collect_retired(std::mutex &mutex, VectorDeque<Retire<T>> &retired,
                       Timepoint timepoint) {
    m_scratch.release();
    Vector<T, scratch_allocator_ref> objects{&m_scratch};
    {
      std::lock_guard lck{mutex};
      while (!retired.empty()) {
        Retire<T> &retire = retired.front();
        if (retire.timepoint <= timepoint) {
          objects.emplace_back(std::move(retired.front().object));
          retired.pop_front();
        } else {
          break;
        }
      }
    }
  }

  void backpressure(TimelineRetireBuffer &buffer, Timepoint completed) {
    Timepoint target;
    {
      std::lock_guard lock{buffer.mutex};
      assert(buffer.completed <= completed);
      buffer.completed = completed;
      assert(buffer.completed <= buffer.retired);
      const uint64_t pending = buffer.retired - buffer.completed;
      if (pending == 0) {
        return;
      }
      target = buffer.completed + BACKPRESSURE_THRESHOLD;
      if (buffer.retired < target) {
        target = buffer.retired;
      }
    }
    Timeline::notify(target);
  }

private:
  Context m_context;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 10>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator m_scratch;
  Vector<TimelineRetireBuffer> m_retireBuffers;
  TimelineBarrier m_barrier;
  std::jthread m_thread;
};

} // namespace strobe::rhi
