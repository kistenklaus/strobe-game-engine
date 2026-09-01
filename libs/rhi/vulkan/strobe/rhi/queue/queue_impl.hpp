#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/core/memory/null_allocator.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/gc/garbage_collector.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/sync/timeline_impl.hpp"
#include "strobe/rhi/sync/timeline_notify_flag.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include "strobe/rhi/utils/pipeline_stage_utils.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/queue.hpp"
#include "strobe/rhi/vulkan/queue_type.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <utility>
#include <vulkan/vulkan_core.h>

#include <tracy/Tracy.hpp>

namespace strobe::rhi {

struct QueueImpl {
public:
  explicit QueueImpl(Timeline timeline, GarbageCollector gc,
                     vulkan::Queue queue, QueueFlags flags)
      : m_timeline(std::move(timeline)), m_gc(std::move(gc)), m_queue(queue),
        m_flags(flags), m_open(&m_batches[0]) {
    m_timeline.install_commit(this, [](void *self, Timepoint timepoint) noexcept {
      static_cast<QueueImpl *>(self)->commit(timepoint);
    });
  }

  QueueImpl(const QueueImpl &) = delete;
  QueueImpl(QueueImpl &&) = delete;
  QueueImpl &operator=(const QueueImpl &) = delete;
  QueueImpl &operator=(QueueImpl &&) = delete;

  ~QueueImpl() noexcept {
    // Object destruction is still required to be externally synchronized with
    // public Queue calls and timeline notification callbacks.
    flush();

    {
      // vkQueueWaitIdle is externally synchronized with every operation on the
      // same VkQueue, just like vkQueueSubmit2 and vkQueuePresentKHR.
      std::lock_guard submitLock{m_submitMutex};
      vulkan::wait_queue_idle(m_queue);
    }

    m_timeline.uninstall_commit();
  }

  QueueFlags flags() const noexcept { return m_flags; }
  uint32_t family() const noexcept { return m_queue.family; }

  void wait(const SwapchainImage &image, PipelineStage stage) {
    auto *img = object_handle_ptr<SwapchainImageImpl>(image);
    SwapchainFrame &frame = img->generation.frame(img->index);
    BinarySemaphore imageAvailable = std::move(frame.imageAvailable);
    assert(imageAvailable);

    std::lock_guard recordLock{m_recordMutex};

    // The semaphore is consumed by the next submission recorded in m_open.
    // Retiring it at the queue's next serial keeps it alive through that wait.
    m_gc.retire(m_timeline.now(), &imageAvailable);
    add_wait_locked(imageAvailable.wait(), stage);
  }

  void wait(const Timepoint &timepoint, PipelineStage stage) noexcept {
    std::lock_guard recordLock{m_recordMutex};
    add_wait_locked(timepoint, stage);
  }

  // Internal API. This has the same ordering contract as before: callers must
  // prevent another thread from inserting a submit between this call and the
  // submit that is intended to consume the signal.
  void signal_on_next_submit(const Timepoint &timepoint,
                             PipelineStage stage) noexcept {
    std::lock_guard recordLock{m_recordMutex};
    add_signal_locked(timepoint, stage);
  }

  Timepoint submit(span<const CommandBuffer> cmds) noexcept {
    for (;;) {
      std::unique_lock recordLock{m_recordMutex};

      if (m_open->submissions.size() == MAX_SUBMIT_BATCH_SIZE) {
        if (m_ready != nullptr) {
          // Both slots are occupied. Do not hold the recording lock while
          // waiting for dependency resolution or the Vulkan queue.
          recordLock.unlock();
          drain_ready_batch();
          continue;
        }

        rotate_locked();
      }

      Timepoint timepoint = m_timeline.advance();

      auto &submission = m_open->submissions.emplace_back(
          timepoint, m_wait, m_signal, m_dependencies, &m_open->bump);

      submission.cmds.resize(cmds.size());
      for (uint32_t i = 0; i < cmds.size(); ++i) {
        submission.cmds[i] = VkCommandBufferSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer =
                object_handle_ptr<CommandBufferImpl>(cmds[i])->cmd.handle,
            .deviceMask = 0,
        };
      }

      m_wait.clear();
      m_signal.clear();
      m_dependencies.clear();

      m_gc.retire(timepoint, cmds);
      return timepoint;
    }
  }

  void present(SwapchainImage image) noexcept {
    Timepoint presentDependency;

    for (;;) {
      std::unique_lock recordLock{m_recordMutex};

      // A presentation closes the current batch. With two fixed slots, the
      // previous closed batch must be drained before another rotation.
      if (m_ready != nullptr) {
        recordLock.unlock();
        drain_ready_batch();
        continue;
      }

      assert(!m_open->presentation);

      auto *img = object_handle_ptr<SwapchainImageImpl>(image);
      auto [presentReady, presentFence] = img->generation.present();

      if (m_open->submissions.empty()) {
        presentDependency = m_timeline.advance();

        add_signal_locked(presentReady.signal());
        m_open->submissions.emplace_back(presentDependency, m_wait, m_signal,
                                         m_dependencies, &m_open->bump);

        m_wait.clear();
        m_signal.clear();
        m_dependencies.clear();
      } else {
        QueueSubmission &submission = m_open->submissions.back();
        submission.add_signal(presentReady.signal());
        presentDependency = submission.timepoint;
      }

      m_open->presentation = QueuePresentation{
          .image = std::move(image),
          .presentReady = std::move(presentReady),
          .presentFence = std::move(presentFence),
      };

      // The closed batch becomes immutable here. Recording can immediately
      // continue in the other fixed-address slot.
      rotate_locked();
      recordLock.unlock();

      // request_commit may wake the GC immediately, so it must happen after
      // publishing the closed batch and after releasing m_recordMutex.
      m_gc.request_commit(presentDependency);
      return;
    }
  }

  void flush() noexcept {
    Timepoint target;

    {
      std::lock_guard recordLock{m_recordMutex};

      // m_open always contains newer queue serials than m_ready.
      if (!m_open->submissions.empty()) {
        target = m_open->submissions.back().timepoint;
      } else if (m_ready != nullptr) {
        target = m_ready->submissions.back().timepoint;
      } else {
        return;
      }
    }

    commit(target);

    // commit(target) may legally return once target's signal operation has
    // been submitted, while another drainer is still inside queue_present.
    // flush() is the stronger barrier and waits for that CPU-side queue work.
    std::lock_guard submitBarrier{m_submitMutex};
  }

private:
  static constexpr std::size_t MAX_SUBMIT_BATCH_SIZE = 16;

  using bump_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 13>;
  using bump_allocator_ref = AllocatorReference<bump_allocator>;

  struct QueueSubmission {
    explicit QueueSubmission(Timepoint timepoint,
                             span<const VkSemaphoreSubmitInfo> wait,
                             span<const VkSemaphoreSubmitInfo> signal,
                             span<const Timepoint> dependencies,
                             bump_allocator_ref alloc)
        : timepoint(std::move(timepoint)), wait(wait, alloc), signal(alloc),
          cmds(alloc), dependencies(dependencies, alloc) {
      // One timeline signal plus one possible presentation signal.
      this->signal.reserve(signal.size() + 2);
      this->signal.emplace_back(VkSemaphoreSubmitInfo{
          .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
          .pNext = nullptr,
          .semaphore =
              TimelineImpl::get_timepoint_semaphore(this->timepoint).handle,
          .value = TimelineImpl::get_timepoint_serial(this->timepoint),
          .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
          .deviceIndex = 0,
      });
      this->signal.append(signal);
    }

    void
    add_signal(vulkan::BinarySemaphore semaphore,
               PipelineStage stage = PipelineStage::all_commands) noexcept {
      const VkPipelineStageFlags2 stageMask = to_vk_pipeline_stage(stage);

      for (auto &signalInfo : signal) {
        if (signalInfo.semaphore == semaphore.handle) {
          signalInfo.stageMask |= stageMask;
          return;
        }
      }

      signal.push_back(VkSemaphoreSubmitInfo{
          .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
          .pNext = nullptr,
          .semaphore = semaphore.handle,
          .value = 0,
          .stageMask = stageMask,
          .deviceIndex = 0,
      });
    }

    Timepoint timepoint;
    Vector<VkSemaphoreSubmitInfo, bump_allocator_ref> wait;
    Vector<VkSemaphoreSubmitInfo, bump_allocator_ref> signal;
    Vector<VkCommandBufferSubmitInfo, bump_allocator_ref> cmds;
    Vector<Timepoint, bump_allocator_ref> dependencies;
  };

  struct QueuePresentation {
    SwapchainImage image;
    BinarySemaphore presentReady;
    Fence presentFence;
  };

  struct QueueBatch {
    // Must be declared before submissions so it is destroyed after every
    // container whose allocator points into it.
    bump_allocator bump;

    SmallVector<QueueSubmission, MAX_SUBMIT_BATCH_SIZE, strobe::NullAllocator>
        submissions;

    std::optional<QueuePresentation> presentation;

    QueueBatch() = default;
    QueueBatch(const QueueBatch &) = delete;
    QueueBatch(QueueBatch &&) = delete;
    QueueBatch &operator=(const QueueBatch &) = delete;
    QueueBatch &operator=(QueueBatch &&) = delete;

    bool reusable() const noexcept {
      return submissions.empty() && !presentation;
    }

    uint64_t first_serial() const noexcept {
      assert(!submissions.empty());
      return TimelineImpl::get_timepoint_serial(submissions.front().timepoint);
    }

    uint64_t last_serial() const noexcept {
      assert(!submissions.empty());
      return TimelineImpl::get_timepoint_serial(submissions.back().timepoint);
    }

    void reset() noexcept {
      // Destroy allocator-aware vectors before releasing their storage.
      submissions.clear();
      presentation.reset();
      bump.release();
    }
  };

  // m_recordMutex must be held.
  void rotate_locked() noexcept {
    assert(m_ready == nullptr);
    assert(!m_open->submissions.empty());

    QueueBatch *const closed = m_open;
    QueueBatch *const next =
        closed == &m_batches[0] ? &m_batches[1] : &m_batches[0];

    assert(next->reusable());

    m_ready = closed;
    m_open = next;
  }

  // m_recordMutex must be held.
  void add_dependency_locked(const Timepoint &timepoint) noexcept {
    // A queue never has to notify itself before submitting its own earlier
    // work. Keeping the actual Vulkan wait is still valid and preserves the
    // requested GPU dependency.
    if (m_timeline.contains(timepoint)) {
      return;
    }

    const VkSemaphore semaphore =
        TimelineImpl::get_timepoint_semaphore(timepoint).handle;

    for (Timepoint &dependency : m_dependencies) {
      if (TimelineImpl::get_timepoint_semaphore(dependency).handle ==
          semaphore) {
        if (dependency < timepoint) {
          dependency = timepoint;
        }
        return;
      }
    }

    m_dependencies.push_back(timepoint);
  }

  // m_recordMutex must be held.
  void add_wait_locked(
      const Timepoint &timepoint,
      PipelineStage stage = PipelineStage::all_commands) noexcept {
    // This must happen before the merge early return: the retained dependency
    // has to track the same maximum value as the Vulkan semaphore wait.
    add_dependency_locked(timepoint);

    const vulkan::TimelineSemaphore semaphore =
        TimelineImpl::get_timepoint_semaphore(timepoint);
    const uint64_t serial =
        TimelineImpl::get_timepoint_serial(timepoint);
    const VkPipelineStageFlags2 stageMask = to_vk_pipeline_stage(stage);

    for (auto &waitInfo : m_wait) {
      if (waitInfo.semaphore == semaphore.handle) {
        waitInfo.stageMask |= stageMask;
        waitInfo.value = std::max(waitInfo.value, serial);
        return;
      }
    }

    m_wait.push_back(VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore.handle,
        .value = serial,
        .stageMask = stageMask,
        .deviceIndex = 0,
    });
  }

  // m_recordMutex must be held.
  void add_wait_locked(
      vulkan::BinarySemaphore semaphore,
      PipelineStage stage = PipelineStage::all_commands) noexcept {
    const VkPipelineStageFlags2 stageMask = to_vk_pipeline_stage(stage);

    for (auto &waitInfo : m_wait) {
      if (waitInfo.semaphore == semaphore.handle) {
        waitInfo.stageMask |= stageMask;
        return;
      }
    }

    m_wait.push_back(VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore.handle,
        .value = 0,
        .stageMask = stageMask,
        .deviceIndex = 0,
    });
  }

  // m_recordMutex must be held.
  void add_signal_locked(
      const Timepoint &timepoint,
      PipelineStage stage = PipelineStage::all_commands) noexcept {
    const vulkan::TimelineSemaphore semaphore =
        TimelineImpl::get_timepoint_semaphore(timepoint);
    const uint64_t serial =
        TimelineImpl::get_timepoint_serial(timepoint);
    const VkPipelineStageFlags2 stageMask = to_vk_pipeline_stage(stage);

    for (auto &signalInfo : m_signal) {
      if (signalInfo.semaphore == semaphore.handle) {
        signalInfo.stageMask |= stageMask;
        signalInfo.value = std::max(signalInfo.value, serial);
        return;
      }
    }

    m_signal.push_back(VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore.handle,
        .value = serial,
        .stageMask = stageMask,
        .deviceIndex = 0,
    });
  }

  // m_recordMutex must be held.
  void add_signal_locked(
      vulkan::BinarySemaphore semaphore,
      PipelineStage stage = PipelineStage::all_commands) noexcept {
    const VkPipelineStageFlags2 stageMask = to_vk_pipeline_stage(stage);

    for (auto &signalInfo : m_signal) {
      if (signalInfo.semaphore == semaphore.handle) {
        signalInfo.stageMask |= stageMask;
        return;
      }
    }

    m_signal.push_back(VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore.handle,
        .value = 0,
        .stageMask = stageMask,
        .deviceIndex = 0,
    });
  }

  void commit(const Timepoint &timepoint) noexcept {
    ZoneScopedN("queue/commit");

    assert(m_timeline.contains(timepoint));
    const uint64_t target =
        TimelineImpl::get_timepoint_serial(timepoint);

    // Hot path: this is an actual submitted watermark, not merely a claimed
    // or requested watermark.
    if (m_submitted.load(std::memory_order_acquire) >= target) {
      return;
    }

    for (;;) {
      if (m_submitted.load(std::memory_order_acquire) >= target) {
        return;
      }

      {
        std::lock_guard recordLock{m_recordMutex};

        if (m_ready == nullptr) {
          // If target has not been submitted and no closed batch exists, it
          // must still be in the open batch.
          assert(!m_open->submissions.empty());
          assert(m_open->first_serial() <= target);
          assert(m_open->last_serial() >= target);
          rotate_locked();
        }
      }

      // This function never holds m_recordMutex while waiting for another
      // producer timeline, the driver, or the queue serialization lock.
      drain_ready_batch();
    }
  }

  void drain_ready_batch() noexcept {
    // Vulkan requires host access to a VkQueue to be externally synchronized.
    // This lock also makes closed batches drain in queue-timeline order.
    std::lock_guard submitLock{m_submitMutex};

    QueueBatch *batch = nullptr;
    {
      std::lock_guard recordLock{m_recordMutex};
      batch = m_ready;
    }

    // Another drainer may have handled the batch while this thread waited for
    // m_submitMutex.
    if (batch == nullptr) {
      return;
    }

    submit_batch(*batch);

    // The batch remains published as m_ready during reset, so recording
    // threads cannot rotate into and reuse its fixed slot prematurely.
    batch->reset();

    {
      std::lock_guard recordLock{m_recordMutex};
      assert(m_ready == batch);
      m_ready = nullptr;
    }
  }

  // m_submitMutex must be held. The batch is immutable and remains at a
  // stable address for the duration of this function.
  void submit_batch(QueueBatch &batch) noexcept {
    ZoneScopedN("queue/submit");
    assert(!batch.submissions.empty());

    std::array<VkSubmitInfo2, MAX_SUBMIT_BATCH_SIZE> submitInfos{};

    for (uint32_t i = 0; i < batch.submissions.size(); ++i) {
      QueueSubmission &submission = batch.submissions[i];

      // Resolve every producer signal before submitting a consumer wait. This
      // is required for binary signals that are subsequently passed to WSI.
      for (const Timepoint &dependency : submission.dependencies) {
        Timeline::notify(dependency, TimelineNotifyFlag::block);
      }

      submitInfos[i] = VkSubmitInfo2{
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
          .pNext = nullptr,
          .flags = 0,
          .waitSemaphoreInfoCount =
              static_cast<uint32_t>(submission.wait.size()),
          .pWaitSemaphoreInfos = submission.wait.data(),
          .commandBufferInfoCount =
              static_cast<uint32_t>(submission.cmds.size()),
          .pCommandBufferInfos = submission.cmds.data(),
          .signalSemaphoreInfoCount =
              static_cast<uint32_t>(submission.signal.size()),
          .pSignalSemaphoreInfos = submission.signal.data(),
      };
    }

    {
      ZoneScopedN("vkQueueSubmit2");
      const VkResult result =
          vkQueueSubmit2(m_queue.handle,
                         static_cast<uint32_t>(batch.submissions.size()),
                         submitInfos.data(), VK_NULL_HANDLE);

      if (result != VK_SUCCESS) {
        vulkan_error(result, "Failed to submit to queue");
      }
    }

    // Publish only after vkQueueSubmit2 accepted the signal operation. This is
    // the property Timeline::notify callers depend on before they may submit a
    // dependent binary-semaphore signal/presentation chain.
    m_submitted.store(batch.last_serial(), std::memory_order_release);

    if (batch.presentation) {
      QueuePresentation &presentation = *batch.presentation;
      auto *img = object_handle_ptr<SwapchainImageImpl>(presentation.image);
      auto *generation =
          object_handle_ptr<SwapchainGenerationImpl>(img->generation);

      generation->debugCounter.fetch_sub(1, std::memory_order_relaxed);

      vulkan::queue_present(
          m_queue, generation->swapchain, img->index,
          {
              .presentReady = presentation.presentReady.wait(),
              .presentFence = presentation.presentFence.fence(),
          });

      m_gc.retire(presentation.presentFence);
      img->consume();
    }
  }

  Timeline m_timeline;
  GarbageCollector m_gc;
  vulkan::Queue m_queue;
  QueueFlags m_flags;

  // Fixed storage is essential: QueueSubmission's vectors retain allocator
  // references into their containing QueueBatch::bump.
  std::array<QueueBatch, 2> m_batches{};
  QueueBatch *m_open = nullptr;
  QueueBatch *m_ready = nullptr;

  // These describe the next logical submission and therefore deliberately do
  // not live in either physical batch slot. A full-batch rotation must not
  // strand a wait or signal in the batch that was just closed.
  Vector<VkSemaphoreSubmitInfo> m_wait;
  Vector<VkSemaphoreSubmitInfo> m_signal;
  Vector<Timepoint> m_dependencies;

  // Highest queue timeline value whose signal operation has actually been
  // accepted by vkQueueSubmit2.
  std::atomic<uint64_t> m_submitted{0};

#ifdef STROBE_RHI_TRACE_LOCKS
  TracyLockableN(std::mutex, m_recordMutex, "Queue-record-mutex");
  TracyLockableN(std::mutex, m_submitMutex, "Queue-submit-mutex");
#else
  std::mutex m_recordMutex{};
  std::mutex m_submitMutex{};
#endif
};

} // namespace strobe::rhi
