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
#include "strobe/rhi/types/pipeline_stage.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include "strobe/rhi/utils/pipeline_stage_utils.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/queue.hpp"
#include "strobe/rhi/vulkan/queue_type.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <mutex>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct QueueImpl {
public:
  explicit QueueImpl(Timeline timeline, GarbageCollector gc,
                     vulkan::Queue queue, QueueFlags flags)
      : m_timeline(std::move(timeline)), m_gc(std::move(gc)), m_queue(queue),
        m_flags(flags), m_submissions() {
    m_timeline.install_commit(this, [](void *self, Timepoint tp) {
      static_cast<QueueImpl *>(self)->commit(tp);
    });
  }
  QueueImpl(const QueueImpl &) = delete;
  QueueImpl(QueueImpl &&) = delete;
  QueueImpl &operator=(const QueueImpl &) = delete;
  QueueImpl &operator=(QueueImpl &&) = delete;
  ~QueueImpl() noexcept {
    close_submit();
    vulkan::wait_queue_idle(m_queue);
    m_timeline.uninstall_commit();
  }

  QueueFlags flags() const noexcept { return m_flags; }
  uint32_t family() const noexcept { return m_queue.family; }

  void wait(const SwapchainImage &image, PipelineStage stage) {
    auto *img = object_handle_ptr<SwapchainImageImpl>(image);
    SwapchainFrame &frame = img->generation.frame(img->index);
    BinarySemaphore imageAvailable = std::move(frame.imageAvailable);
    assert(imageAvailable);
    std::lock_guard lck{m_mutex};
    m_gc.retire(m_timeline.now(), &imageAvailable);
    add_wait(imageAvailable.wait(), stage);
  }

  void wait(const Timepoint &timepoint, PipelineStage stage) noexcept {
    // Required because of presentation, binary semaphores.
    Timeline::notify(timepoint);

    std::lock_guard lck{m_mutex};
    add_wait(timepoint, stage);
  }

  // internal!
  void signal_on_next_submit(const Timepoint &timepoint,
                             PipelineStage stage) noexcept {
    std::lock_guard lck{m_mutex};
    add_signal(timepoint, stage);
  }

  Timepoint submit(span<const CommandBuffer> cmds) noexcept {
    std::lock_guard lck{m_mutex};
    if (m_submissions.size() == MAX_SUBMIT_BATCH_SIZE) {
      close_submit();
    }
    Timepoint timepoint = m_timeline.advance();
    auto &submission =
        m_submissions.emplace_back(timepoint, m_wait, m_signal, &m_bump);
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
    m_gc.retire(timepoint, cmds);
    return timepoint;
  }

  void present(SwapchainImage image) noexcept {
    std::lock_guard lck{m_mutex};
    if (m_pendingPresent) {
      close_submit();
    }
    auto *img = object_handle_ptr<SwapchainImageImpl>(image);
    auto [presentReady, presentFence] = img->generation.present();
    Timepoint presentDependency;
    if (m_submissions.empty()) {
      presentDependency = m_timeline.advance();
      add_signal(presentReady.signal());
      m_submissions.emplace_back(presentDependency, m_wait, m_signal, &m_bump);
      m_wait.clear();
      m_signal.clear();
    } else {
      auto &sub = m_submissions.back();
      sub.add_signal(presentReady.signal());
      presentDependency = sub.timepoint;
    }
    assert(!m_pendingPresent);
    m_pendingPresent = QueuePresentation{
        .image = std::move(image),
        .presentReady = std::move(presentReady),
        .presentFence = std::move(presentFence),
    };
    m_gc.request_commit(presentDependency);
  }

  void flush() noexcept {
    std::lock_guard lck{m_mutex};
    close_submit();
  };

private:
  static constexpr size_t MAX_SUBMIT_BATCH_SIZE = 16;

  using bump_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 13>;
  using bump_allocator_ref = AllocatorReference<bump_allocator>;

  struct QueueSubmission {
    explicit QueueSubmission(Timepoint timepoint,
                             span<const VkSemaphoreSubmitInfo> wait,
                             span<const VkSemaphoreSubmitInfo> signal,
                             bump_allocator_ref alloc)
        : timepoint(timepoint), wait(wait, alloc), signal(alloc), cmds(alloc) {
      this->signal.reserve(signal.size() +
                           2); // one extra space for possible present signals.
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
      auto stageMask = to_vk_pipeline_stage(stage);
      for (auto &signal : this->signal) {
        if (signal.semaphore == semaphore.handle) {
          signal.stageMask |= stageMask;
          return;
        }
      }
      this->signal.push_back(VkSemaphoreSubmitInfo{
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
  };

  struct QueuePresentation {
    SwapchainImage image;
    BinarySemaphore presentReady;
    Fence presentFence;
  };

  // externally synchronized
  void add_wait(const Timepoint &timepoint,
                PipelineStage stage = PipelineStage::all_commands) noexcept {
    const vulkan::TimelineSemaphore sem =
        TimelineImpl::get_timepoint_semaphore(timepoint);
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
    const auto stageMask = to_vk_pipeline_stage(stage);
    for (auto &wait : m_wait) {
      if (wait.semaphore == sem.handle) {
        wait.stageMask |= stageMask;
        wait.value = std::max(wait.value, serial);
        return;
      }
    }
    m_wait.push_back(VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = TimelineImpl::get_timepoint_semaphore(timepoint).handle,
        .value = TimelineImpl::get_timepoint_serial(timepoint),
        .stageMask = to_vk_pipeline_stage(stage),
        .deviceIndex = 0,
    });
  }

  // externally synchronized
  void add_wait(vulkan::BinarySemaphore semaphore,
                PipelineStage stage = PipelineStage::all_commands) noexcept {
    auto stageMask = to_vk_pipeline_stage(stage);
    for (auto &wait : m_wait) {
      if (wait.semaphore == semaphore.handle) {
        wait.stageMask |= stageMask;
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

  // externally synchronized
  void add_signal(const Timepoint &timepoint,
                  PipelineStage stage = PipelineStage::all_commands) noexcept {
    const vulkan::TimelineSemaphore sem =
        TimelineImpl::get_timepoint_semaphore(timepoint);
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
    const auto stageMask = to_vk_pipeline_stage(stage);
    for (auto &signal : m_signal) {
      if (signal.semaphore == sem.handle) {
        signal.stageMask |= stageMask;
        signal.value = std::max(signal.value, serial);
        return;
      }
    }
    m_signal.push_back(VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = sem.handle,
        .value = serial,
        .stageMask = stageMask,
        .deviceIndex = 0,
    });
  }

  // externally synchronized
  void add_signal(vulkan::BinarySemaphore semaphore,
                  PipelineStage stage = PipelineStage::all_commands) noexcept {
    auto stageMask = to_vk_pipeline_stage(stage);
    for (auto &signal : m_signal) {
      if (signal.semaphore == semaphore.handle) {
        signal.stageMask |= stageMask;
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

  void commit(Timepoint timepoint) noexcept {
    ZoneScopedN("queue/commit");
    std::lock_guard lck{m_mutex};
    if (m_submissions.empty() && !m_pendingPresent) {
      return;
    }
    if (timepoint < m_submissions.front().timepoint) {
      return;
    }
    close_submit();
  }

  // externally synchronized!
  void close_submit() noexcept {
    ZoneScopedN("queue/submit");
    if (!m_submissions.empty()) {

      std::array<VkSubmitInfo2, MAX_SUBMIT_BATCH_SIZE> submitInfos;
      for (uint32_t i = 0; i < m_submissions.size(); ++i) {
        auto &submission = m_submissions[i];

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
        VkResult result = vkQueueSubmit2(m_queue.handle, m_submissions.size(),
                                         submitInfos.data(), VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
          vulkan_error(result, "Failed to submit to queue");
        }
      }
      m_submissions.clear();
      m_bump.release();
    }

    if (m_pendingPresent) {
      auto *img =
          object_handle_ptr<SwapchainImageImpl>(m_pendingPresent->image);
      auto *gen = object_handle_ptr<SwapchainGenerationImpl>(img->generation);
      gen->debugCounter.fetch_sub(1, std::memory_order_relaxed);
      vulkan::queue_present(
          m_queue, gen->swapchain, img->index,
          {
              .presentReady = m_pendingPresent->presentReady.wait(),
              .presentFence = m_pendingPresent->presentFence.fence(),
          });
      m_gc.retire(m_pendingPresent->presentFence);
      img->consume();
      m_pendingPresent.reset();
    }
  }

  Timeline m_timeline;
  GarbageCollector m_gc;
  vulkan::Queue m_queue;
  QueueFlags m_flags;
  bump_allocator m_bump;
  Vector<VkSemaphoreSubmitInfo> m_wait;
  Vector<VkSemaphoreSubmitInfo> m_signal;

  SmallVector<QueueSubmission, MAX_SUBMIT_BATCH_SIZE, strobe::NullAllocator>
      m_submissions;

  std::optional<QueuePresentation> m_pendingPresent;

#ifdef STROBE_RHI_TRACE_LOCKS
  TracyLockableN(std::mutex, m_mutex, "Queue-mutex");
#else
  std::mutex m_mutex{};
#endif
};

} // namespace strobe::rhi
