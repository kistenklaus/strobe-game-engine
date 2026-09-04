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
    m_timeline.install_commit(
        this, [](void *self, Timepoint timepoint) noexcept {
          static_cast<QueueImpl *>(self)->commit(timepoint);
        });
  }

  QueueImpl(const QueueImpl &) = delete;
  QueueImpl(QueueImpl &&) = delete;
  QueueImpl &operator=(const QueueImpl &) = delete;
  QueueImpl &operator=(QueueImpl &&) = delete;

  ~QueueImpl() noexcept {
    flush();
    {
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
    m_gc.retire(m_timeline.now(), &imageAvailable);
    add_wait(imageAvailable.wait(), stage);
  }

  void wait(const Timepoint &timepoint, PipelineStage stage) noexcept {
    if (!timepoint) {
      return;
    }
    std::lock_guard recordLock{m_recordMutex};
    add_wait(timepoint, stage);
  }

  void signal_on_next_submit(const Timepoint &timepoint,
                             PipelineStage stage) noexcept {
    std::lock_guard recordLock{m_recordMutex};
    add_signal(timepoint, stage);
  }

  Timepoint submit(span<const CommandBuffer> cmds) noexcept {
    Timepoint dmaReady{};
    for (auto &cmd : cmds) {
      dmaReady &= object_handle_ptr<CommandBufferImpl>(cmd)->dma_ready;
    }

    while (true) {
      std::unique_lock recordLock{m_recordMutex};
      if (m_open->submissions.size() == MAX_SUBMIT_BATCH_SIZE) {
        if (m_ready != nullptr) {
          recordLock.unlock();
          drain_ready_batch();
          continue;
        }
        rotate();
      }
      if (dmaReady) {
        add_wait(dmaReady, PipelineStage::all_commands);
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

    while (true) {
      std::unique_lock recordLock{m_recordMutex};
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
        add_signal(presentReady.signal());
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
      rotate();
      recordLock.unlock();
      m_gc.request_commit(presentDependency);
      return;
    }
  }

  void flush() noexcept {
    Timepoint target;
    {
      std::lock_guard recordLock{m_recordMutex};
      if (!m_open->submissions.empty()) {
        target = m_open->submissions.back().timepoint;
      } else if (m_ready != nullptr) {
        target = m_ready->submissions.back().timepoint;
      } else {
        return;
      }
    }
    commit(target);
    // wait for completion, commit may complete on another thread.
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
      submissions.clear();
      presentation.reset();
      bump.release();
    }

    bump_allocator bump;
    SmallVector<QueueSubmission, MAX_SUBMIT_BATCH_SIZE, strobe::NullAllocator>
        submissions;
    std::optional<QueuePresentation> presentation;
  };

  // m_recordMutex must be held.
  void rotate() noexcept {
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
  void add_dependency(const Timepoint &timepoint) noexcept {
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
  void add_wait(const Timepoint &timepoint,
                PipelineStage stage = PipelineStage::all_commands) noexcept {
    add_dependency(timepoint);
    const vulkan::TimelineSemaphore semaphore =
        TimelineImpl::get_timepoint_semaphore(timepoint);
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
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
  void add_wait(vulkan::BinarySemaphore semaphore,
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
  void add_signal(const Timepoint &timepoint,
                  PipelineStage stage = PipelineStage::all_commands) noexcept {
    const vulkan::TimelineSemaphore semaphore =
        TimelineImpl::get_timepoint_semaphore(timepoint);
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
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
  void add_signal(vulkan::BinarySemaphore semaphore,
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
    const uint64_t target = TimelineImpl::get_timepoint_serial(timepoint);
    if (m_submitted.load(std::memory_order_acquire) >= target) {
      return;
    }
    while (true) {
      if (m_submitted.load(std::memory_order_acquire) >= target) {
        return;
      }
      {
        std::lock_guard recordLock{m_recordMutex};
        if (m_ready == nullptr) {
          assert(!m_open->submissions.empty());
          assert(m_open->first_serial() <= target);
          assert(m_open->last_serial() >= target);
          rotate();
        }
      }
      drain_ready_batch();
    }
  }

  void drain_ready_batch() noexcept {
    std::lock_guard submitLock{m_submitMutex};

    QueueBatch *batch = nullptr;
    {
      std::lock_guard recordLock{m_recordMutex};
      batch = m_ready;
    }
    if (batch == nullptr) {
      return;
    }

    submit_batch(*batch);
    batch->reset();
    {
      std::lock_guard recordLock{m_recordMutex};
      assert(m_ready == batch);
      m_ready = nullptr;
    }
  }

  // m_submitMutex must be held. The batch is immutable and pined.
  void submit_batch(QueueBatch &batch) noexcept {
    ZoneScopedN("queue/submit");
    assert(!batch.submissions.empty());
    std::array<VkSubmitInfo2, MAX_SUBMIT_BATCH_SIZE> submitInfos{};
    for (uint32_t i = 0; i < batch.submissions.size(); ++i) {
      QueueSubmission &submission = batch.submissions[i];
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
#ifdef STROBE_RHI_TRACE_VK
      ZoneScopedN("vkQueueSubmit2");
#endif
      const VkResult result = vkQueueSubmit2(
          m_queue.handle, static_cast<uint32_t>(batch.submissions.size()),
          submitInfos.data(), VK_NULL_HANDLE);

      if (result != VK_SUCCESS) {
        vulkan_error(result, "Failed to submit to queue");
      }
    }
    m_submitted.store(batch.last_serial(), std::memory_order_release);
    if (batch.presentation) {
      QueuePresentation &presentation = *batch.presentation;
      auto *img = object_handle_ptr<SwapchainImageImpl>(presentation.image);
      auto *generation =
          object_handle_ptr<SwapchainGenerationImpl>(img->generation);
      generation->debugCounter.fetch_sub(1, std::memory_order_relaxed);
      {
        std::lock_guard lck{generation->mutex};
        vulkan::queue_present(
            m_queue, generation->swapchain, img->index,
            {
                .presentReady = presentation.presentReady.wait(),
                .presentFence = presentation.presentFence.fence(),
            });
      }
      m_gc.retire(presentation.presentFence);
      img->consume();
    }
  }

  Timeline m_timeline;
  GarbageCollector m_gc;
  vulkan::Queue m_queue;
  QueueFlags m_flags;

  std::array<QueueBatch, 2> m_batches{};
  QueueBatch *m_open = nullptr;
  QueueBatch *m_ready = nullptr;

  Vector<VkSemaphoreSubmitInfo> m_wait;
  Vector<VkSemaphoreSubmitInfo> m_signal;
  Vector<Timepoint> m_dependencies;

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
