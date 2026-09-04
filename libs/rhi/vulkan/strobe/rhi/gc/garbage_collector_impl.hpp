#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/containers/vector_deque.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/sync/fence.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/sync/timeline_barrier.hpp"
#include "strobe/rhi/sync/timeline_impl.hpp"
#include "strobe/rhi/sync/timeline_notify_flag.hpp"
#include <cassert>
#include <common/TracySystem.hpp>
#include <cstdint>
#include <limits>
#include <mutex>
#include <stop_token>
#include <thread>
#include <tracy/Tracy.hpp>

namespace strobe::rhi {

struct GarbageCollectorImpl {
  // If the amount of pending timepoints falls below this
  // threshold the timeline is shall be notified in timely manner.
  // " we call retired but not yet completed timepoints pending.
  static constexpr uint64_t BACKPRESSURE_THRESHOLD = 2;

  explicit GarbageCollectorImpl(Context context, span<Timeline> timelines,
                                strobe::rhi::allocator_ref alloc)
      : m_context(std::move(context)), m_retireBuffers(timelines.size()),
        m_barrier(m_context, timelines, alloc) {
    for (uint32_t i = 0; i < timelines.size(); ++i) {
      m_retireBuffers[i].timeline = timelines[i];
      m_retireBuffers[i].completed = {};
      m_retireBuffers[i].retired = {};
    }
    m_thread = std::jthread(&GarbageCollectorImpl::gc_main, this);
  }

  GarbageCollectorImpl(const GarbageCollectorImpl &) = delete;
  GarbageCollectorImpl(GarbageCollectorImpl &&) = delete;
  GarbageCollectorImpl &operator=(const GarbageCollectorImpl &) = delete;
  GarbageCollectorImpl &operator=(GarbageCollectorImpl &&) = delete;
  ~GarbageCollectorImpl() noexcept = default;

  void request_commit(Timepoint timepoint) noexcept {
    uint32_t timelineIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < m_retireBuffers.size(); ++i) {
      if (m_retireBuffers[i].timeline.contains(timepoint)) {
        timelineIndex = i;
        break;
      }
    }
    assert(timelineIndex != std::numeric_limits<uint32_t>::max());
    auto &buffer = m_retireBuffers[timelineIndex];
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);

    bool wake = false;
    {
      std::lock_guard lock{buffer.mutex};
      if (serial <= buffer.completed) {
        return;
      }
      if (!buffer.requestedCommit || buffer.requestedCommit < serial) {
        buffer.requestedCommit = serial;
        wake = true;
      }
    }
    if (wake) {
      ZoneScopedN("gc/request-commit");
      m_barrier.notify();
    }
  }

  // Retire a timepoint, might initally feel weird, but essentially
  // registers the timepoint to the gc for backpressure.
  void retire(Timepoint timepoint) {
    uint32_t timelineIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < m_retireBuffers.size(); ++i) {
      if (m_retireBuffers[i].timeline.contains(timepoint)) {
        timelineIndex = i;
        break;
      }
    }
    assert(timelineIndex != std::numeric_limits<uint32_t>::max());
    auto &buffer = m_retireBuffers[timelineIndex];
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
    bool wake;
    {
      std::lock_guard lock{buffer.mutex};
      assert(buffer.retired <= serial);
      if (buffer.retired == serial) {
        return;
      }
      buffer.retired = serial;
      const uint64_t pending = buffer.retired - buffer.completed;
      wake = pending <= BACKPRESSURE_THRESHOLD;
    }
    if (wake) {
      ZoneScopedN("gc/wake");
      m_barrier.notify();
    }
  }

  void retire(Timepoint timepoint, span<const BinarySemaphore> sems) {
    uint32_t timelineIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < m_retireBuffers.size(); ++i) {
      if (m_retireBuffers[i].timeline.contains(timepoint)) {
        timelineIndex = i;
        break;
      }
    }
    assert(timelineIndex != std::numeric_limits<uint32_t>::max());
    auto &buffer = m_retireBuffers[timelineIndex];
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
    bool wake;
    {
      std::lock_guard lock{buffer.mutex};
      assert(buffer.retired <= serial);
      for (const BinarySemaphore &sem : sems) {
        buffer.retiredSems.emplace_back(serial, sem);
      }
      buffer.retired = serial;
      const uint64_t pending = buffer.retired - buffer.completed;
      wake = pending <= BACKPRESSURE_THRESHOLD;
    }
    if (wake) {
      ZoneScopedN("gc/wake");
      m_barrier.notify();
    }
  }

  void retire(Timepoint timepoint, span<const CommandBuffer> cmds) {
    uint32_t timelineIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < m_retireBuffers.size(); ++i) {
      if (m_retireBuffers[i].timeline.contains(timepoint)) {
        timelineIndex = i;
        break;
      }
    }
    assert(timelineIndex != std::numeric_limits<uint32_t>::max());
    auto &buffer = m_retireBuffers[timelineIndex];
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
    bool wake;
    {
      std::lock_guard lock{buffer.mutex};
      assert(buffer.retired <= serial);
      for (const CommandBuffer &cmd : cmds) {
        buffer.retiredCmds.emplace_back(serial, cmd);
      }
      buffer.retired = serial;
      const uint64_t pending = buffer.retired - buffer.completed;
      wake = pending <= BACKPRESSURE_THRESHOLD;
    }
    if (wake) {
      ZoneScopedN("gc/wake");
      m_barrier.notify();
    }
  }

  void retire(Fence fence) {
    std::lock_guard lck{m_retiredFencesMutex};
    m_retiredFences.emplace_back(std::move(fence));
    m_barrier.notify();
  }

  void gc_main(std::stop_token st) {
    tracy::SetThreadName("rhi-gc");

    std::stop_callback stop_callback(
        st, [barrier = &m_barrier] { barrier->notify(); });

    while (!st.stop_requested()) {
      auto [timelineIndex, timepoint] = m_barrier.wait_any();
      if (st.stop_requested()) {
        break;
      }
      if (timepoint) {
        const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
        auto &retireBuffer = m_retireBuffers[timelineIndex];
        collect(retireBuffer, serial);
        retireBuffer.timeline.complete(timepoint);
        backpressure(retireBuffer, serial);
      } else { // invalid timepoint; just apply backpressure to all timelines
        for (uint32_t i = 0; i < m_retireBuffers.size(); ++i) {
          backpressure(m_retireBuffers[i], m_retireBuffers[i].completed);
        }
      }
      for (auto &buffer : m_retireBuffers) {
        commit_requested(buffer);
      }
      collect_fences();
    }
  }

private:
  template <typename T> struct Retire {
    uint64_t serial;
    T object;
  };

  struct TimelineRetireBuffer {
    Timeline timeline;

#ifdef STROBE_RHI_TRACE_LOCKS
    TracyLockableN(std::mutex, mutex, "GC-fence");
#else
    std::mutex mutex{};
#endif
    VectorDeque<Retire<CommandBuffer>> retiredCmds;
    VectorDeque<Retire<BinarySemaphore>> retiredSems;
    uint64_t completed = 0;
    uint64_t retired = 0;

    uint64_t requestedCommit = 0;
  };

  void collect(TimelineRetireBuffer &retireBuffer, uint64_t serial) {
    collect_retired(retireBuffer.mutex, retireBuffer.retiredCmds, serial);
    collect_retired(retireBuffer.mutex, retireBuffer.retiredSems, serial);
  }

  void collect_retired(auto &mutex, VectorDeque<Retire<CommandBuffer>> &retired,
                       uint64_t serial) {
    m_scratch.release();
    Vector<CommandBuffer, scratch_allocator_ref> objects{&m_scratch};
    objects.reserve(128);
    {
      std::lock_guard lck{mutex};
      if (retired.empty()) {
        return;
      }
      ZoneScopedN("gc/collect-cmds");
      while (!retired.empty()) {
        Retire<CommandBuffer> &retire = retired.front();
        if (retire.serial <= serial) {
          objects.emplace_back(std::move(retired.front().object));
          retired.pop_front();
        } else {
          break;
        }
      }
    }
  }
  void collect_retired(auto &mutex,
                       VectorDeque<Retire<BinarySemaphore>> &retired,
                       uint64_t serial) {
    m_scratch.release();
    Vector<BinarySemaphore, scratch_allocator_ref> objects{&m_scratch};
    objects.reserve(128);
    {
      std::lock_guard lck{mutex};
      if (retired.empty()) {
        return;
      }
      ZoneScopedN("gc/collect-sems");
      while (!retired.empty()) {
        Retire<BinarySemaphore> &retire = retired.front();
        if (retire.serial <= serial) {
          objects.emplace_back(std::move(retired.front().object));
          retired.pop_front();
        } else {
          break;
        }
      }
    }
  }

  void backpressure(TimelineRetireBuffer &buffer, uint64_t completed) {
    uint64_t target;
    {
      std::lock_guard lock{buffer.mutex};
      assert(buffer.completed <= completed);
      buffer.completed = completed;
      if (buffer.retired <= buffer.completed) {
        return;
      }
      target = buffer.completed + BACKPRESSURE_THRESHOLD;
      if (buffer.retired < target) {
        target = buffer.retired;
      }
      const uint64_t lastClosedSerial = buffer.timeline.serial() - 1;
      if (target > lastClosedSerial) {
        target = lastClosedSerial;
      }
      if (target <= buffer.completed) {
        return;
      }
    }
    buffer.timeline.notify(target, TimelineNotifyFlag::backpressure);
  }

  void collect_fences() {
    m_scratch.release();
    Vector<Fence, scratch_allocator_ref> objects{&m_scratch};
    objects.reserve(128);
    {
      std::lock_guard lck{m_retiredFencesMutex};
      while (!m_retiredFences.empty()) {
        auto &fence = m_retiredFences.front();
        if (fence.signaled()) {
          objects.emplace_back(std::move(fence));
          m_retiredFences.pop_front();
        } else {
          break;
        }
      }
    }
  }
  void commit_requested(TimelineRetireBuffer &buffer) {
    uint64_t target;
    {
      std::lock_guard lock{buffer.mutex};
      if (!buffer.requestedCommit) {
        return;
      }
      target = buffer.requestedCommit;
      buffer.requestedCommit = {};
    }
    ZoneScopedN("gc/commit-requested");
    buffer.timeline.notify(target, TimelineNotifyFlag::backpressure);
  }

private:
  Context m_context;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 10>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator m_scratch;
  Vector<TimelineRetireBuffer> m_retireBuffers;

#ifdef STROBE_RHI_TRACE_LOCKS
  TracyLockableN(std::mutex, m_retiredFencesMutex, "GC-fence");
#else
  std::mutex m_retiredFencesMutex;
#endif

  VectorDeque<Fence> m_retiredFences;
  TimelineBarrier m_barrier;
  std::jthread m_thread;
};

} // namespace strobe::rhi
