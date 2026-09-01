#pragma once

#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/queue/queue_impl.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"

namespace strobe::rhi {

struct DMAImpl {
public:
  explicit DMAImpl(Timeline timeline, GarbageCollector gc, Queue queue,
                   CommandPool cmdpool) noexcept
      : m_timeline(std::move(timeline)), m_gc(std::move(gc)),
        m_queue(std::move(queue)), m_cmdpool(std::move(cmdpool)) {

    m_committed = m_timeline.epoch();
    begin_cmd();
    m_timeline.install_commit(this,
                              [](void *ptr, Timepoint timepoint) noexcept {
                                static_cast<DMAImpl *>(ptr)->commit(timepoint);
                              });
  }
  ~DMAImpl() noexcept {
    m_timeline.uninstall_commit();
    std::lock_guard lock{m_mutex};
    if (m_count != 0) {
      end_cmd();
    }
  }

  Timepoint async_copy(BufferOffset dst, BufferOffset src, uint64_t size) {
    std::lock_guard lck{m_mutex};
    m_cmd.copy_buffer(dst, src, size);
    return step();
  }

  Timepoint async_upload(BufferOffset dst, void *src, uint64_t size) {
    std::lock_guard lck{m_mutex};
    m_cmd.update(dst, src, size);
    return step();
  }

private:
  // externally synchronized
  Timepoint step() {
    Timepoint timepoint = m_timeline.advance();
    m_count += 1;
    m_gc.retire(timepoint);
    return timepoint;
  }
  // internally synchronized
  void commit(const Timepoint &timepoint) noexcept {
    std::lock_guard lck{m_mutex};
    if (m_count == 0) {
      return;
    }
    if (m_committed >= timepoint) {
      return; // no need to submit this timepoint again.
    }
    m_committed = timepoint;
    end_cmd();
    begin_cmd();
  }

  void end_cmd() {
    m_cmd.end();
    object_handle_ptr<QueueImpl>(m_queue)->signal_on_next_submit(
        m_committed, PipelineStage::transfer);
    m_queue.submit(&m_cmd);
    m_queue.flush();
    m_count = 0;
  }

  void begin_cmd() {
    m_cmd = m_cmdpool.alloc();
    m_cmd.begin();
  }

  Timeline m_timeline;
  GarbageCollector m_gc;
  Queue m_queue;
  CommandPool m_cmdpool;
  uint32_t m_count = 0;
  Timepoint m_committed;

  CommandBuffer m_cmd{};
  std::mutex m_mutex{};
};

} // namespace strobe::rhi
