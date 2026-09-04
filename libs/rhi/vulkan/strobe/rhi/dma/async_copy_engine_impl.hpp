#pragma once

#include "strobe/rhi/dma/async_transfer_cmd.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/queue/queue_impl.hpp"
#include "strobe/rhi/sync/timeline.hpp"

#include <cassert>
#include <mutex>
#include <tracy/Tracy.hpp>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct AsyncCopyEngineImpl {
  friend class AsyncTransferCmd;

public:
  explicit AsyncCopyEngineImpl(Timeline timeline, GarbageCollector gc,
                               Queue queue, CommandPool cmdpool,
                               strobe::rhi::allocator_ref alloc) noexcept
      : m_timeline(std::move(timeline)), m_gc(std::move(gc)),
        m_queue(std::move(queue)), m_cmdpool(std::move(cmdpool)),
        m_committed({}), m_alloc(alloc) {
    m_open = make_recording_batch();
    m_spare = make_recording_batch();
    m_timeline.install_commit(
        this, [](void *ptr, Timepoint timepoint) noexcept {
          static_cast<AsyncCopyEngineImpl *>(ptr)->commit(timepoint);
        });
  }

  AsyncCopyEngineImpl(const AsyncCopyEngineImpl &) = delete;
  AsyncCopyEngineImpl(AsyncCopyEngineImpl &&) = delete;
  AsyncCopyEngineImpl &operator=(const AsyncCopyEngineImpl &) = delete;
  AsyncCopyEngineImpl &operator=(AsyncCopyEngineImpl &&) = delete;

  ~AsyncCopyEngineImpl() noexcept {
    m_timeline.uninstall_commit();
    std::lock_guard commitLock{m_commitMutex};
    Batch closed;
    bool submitClosed = false;
    {
      std::lock_guard recordLock{m_recordMutex};
      if (m_open.count != 0) {
        m_open.cmd.end();
        closed = std::move(m_open);
        m_open = {};
        submitClosed = true;
      } else {
        m_open.cmd.end();
        m_open = {};
      }
      m_spare.cmd.end();
      m_spare = {};
    }
    if (submitClosed) {
      submit_closed(closed);
    }
  }

  AsyncTransferCmd async_cmd() {
    return AsyncTransferCmd{std::unique_lock{m_recordMutex}, this};
  }

private:
  struct Batch {
    CommandBuffer cmd{};
    Timepoint last{};
    uint32_t count = 0;
  };

  Batch make_recording_batch() {
    Batch batch;
    batch.cmd = m_cmdpool.alloc();
    batch.cmd.begin();
    return batch;
  }

  Timepoint step_locked() {
    assert(m_open.count != UINT32_MAX);
    Timepoint timepoint = m_timeline.advance();
    m_open.last = timepoint;
    ++m_open.count;
    m_gc.retire(timepoint);
    return timepoint;
  }

  // Serialized by m_commitMutex.
  void commit(const Timepoint &requested) noexcept {
    ZoneScopedN("dma/commit");
    std::lock_guard commitLock{m_commitMutex};
    if (m_committed >= requested) {
      return;
    }
    Batch closed;
    {
      ZoneScopedN("dma/rotate");
      std::lock_guard recordLock{m_recordMutex};
      assert(m_open.count != 0);
      assert(m_open.last >= requested);
      assert(m_spare.count == 0);
      assert(!m_spare.last);
      m_open.cmd.end();
      closed = std::move(m_open);
      m_open = std::move(m_spare);
      m_spare = {};
    }

    submit_closed(closed);

    {
      ZoneScopedN("dma/refill-spare");
      std::lock_guard recordLock{m_recordMutex};
      m_spare = make_recording_batch();
    }
  }

  void submit_closed(Batch &closed) noexcept {
    assert(closed.count != 0);
    assert(closed.last);
    assert(m_committed < closed.last);
    auto *queue = object_handle_ptr<QueueImpl>(m_queue);
    queue->signal_on_next_submit(closed.last, PipelineStage::transfer);
    m_queue.submit(&closed.cmd);
    m_queue.flush();

    m_committed = closed.last;
  }

private:
  Timeline m_timeline;
  GarbageCollector m_gc;
  Queue m_queue;
  CommandPool m_cmdpool;

  // open current begin recorded
  Batch m_open;
  // already allocated and begun, but constains no commands.
  Batch m_spare;

  Timepoint m_committed;

#ifdef STROBE_RHI_TRACE_LOCKS
  TracyLockableN(std::mutex, m_recordMutex, "DMA-record-mutex");
  TracyLockableN(std::mutex, m_commitMutex, "DMA-commit-mutex");
#else
  std::mutex m_recordMutex;
  std::mutex m_commitMutex;
#endif

  [[no_unique_address]] strobe::rhi::allocator_ref m_alloc;
};

} // namespace strobe::rhi
