#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/core/memory/null_allocator.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/gc/garbage_collector.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/sync/timepoint.hpp"
#include "strobe/rhi/vulkan/queue_type.hpp"

namespace strobe::rhi {

struct QueueImpl : public Timeline {

  explicit QueueImpl(Context context, GarbageCollector gc, vulkan::Queue queue)
      : Timeline(std::move(context)), m_gc(std::move(gc)), m_queue(queue) {}
  QueueImpl(const QueueImpl &) = delete;
  QueueImpl(QueueImpl &&) = delete;
  QueueImpl &operator=(const QueueImpl &) = delete;
  QueueImpl &operator=(QueueImpl &&) = delete;
  ~QueueImpl() noexcept override {}

  void wait(Timepoint timepoint) noexcept {
    if (timepoint.weak_poll()) {
      return; // already completed no need to wait on it.
    }
    m_batch.openWaits.push_back(timepoint); // TODO merge with existing waits.
  }

  void submit(span<const CommandBuffer> cmds) {
    Timepoint timepoint = advance();
    m_gc.retire(timepoint, cmds);
  }

private:
  static constexpr size_t MAX_SUBMIT_BATCH_SIZE = 16;
  using bump_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 10>;
  using bump_allocator_ref = AllocatorReference<bump_allocator>;
  struct QueueBatch {
    Timepoint begin;

    SmallVector<VkSubmitInfo2, MAX_SUBMIT_BATCH_SIZE, strobe::NullAllocator>
        closedSubmits;

    SmallVector<Timepoint, 16, strobe::NullAllocator> openWaits;
    bump_allocator m_scratch;
  };

  void commit(Timepoint timepoint) final override {
    // submit batch (would you commit the complete batch?)
  }

  GarbageCollector m_gc;
  vulkan::Queue m_queue;

  std::mutex m_mutex;
  mutable QueueBatch m_batch;
};

} // namespace strobe::rhi
