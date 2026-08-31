#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/core/memory/null_allocator.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/cmd/command_buffer_impl.hpp"
#include "strobe/rhi/gc/garbage_collector.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/sync/fence_impl.hpp"
#include "strobe/rhi/sync/timeline.hpp"
#include "strobe/rhi/sync/timeline_impl.hpp"
#include "strobe/rhi/sync/timepoint.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include "strobe/rhi/utils/pipeline_stage_utils.hpp"
#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/queue.hpp"
#include "strobe/rhi/vulkan/queue_type.hpp"
#include <algorithm>
#include <array>
#include <fmt/ostream.h>
#include <mutex>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct QueueImpl {
public:
  explicit QueueImpl(Timeline timeline, GarbageCollector gc,
                     vulkan::Queue queue, QueueFlags flags)
      : m_timeline(std::move(timeline)), m_gc(std::move(gc)), m_queue(queue),
        m_flags(flags), m_submissions() {
    m_timeline.set_commit_callback(this, [](void *self, Timepoint tp) {
      static_cast<QueueImpl *>(self)->commit(tp);
    });
    m_timeline.advance(); // inital step.
  }
  QueueImpl(const QueueImpl &) = delete;
  QueueImpl(QueueImpl &&) = delete;
  QueueImpl &operator=(const QueueImpl &) = delete;
  QueueImpl &operator=(QueueImpl &&) = delete;
  ~QueueImpl() noexcept {
    submit_all();
    vulkan::wait_queue_idle(m_queue);
    m_timeline.clear_callback();
  }

  QueueFlags flags() const noexcept { return m_flags; }

  void wait(SwapchainImage image, PipelineStage stage) {
    auto *img = object_handle_ptr<SwapchainImageImpl>(image);
    SwapchainFrame &frame = img->generation.frame(img->index);
    BinarySemaphore imageAvailable = std::move(frame.imageAvailable);
    assert(imageAvailable);

    std::lock_guard lck{m_mutex};
    m_gc.retire(m_timeline.now(), &imageAvailable);
    m_wait.push_back(VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = imageAvailable.wait().handle,
        .value = 0,
        .stageMask = to_vk_pipeline_stage(stage),
        .deviceIndex = 0,
    });
  }

  void wait(Timepoint timepoint, PipelineStage stage) noexcept {
    const vulkan::TimelineSemaphore timeline =
        TimelineImpl::get_timepoint_semaphore(timepoint);
    const uint64_t serial = TimelineImpl::get_timepoint_serial(timepoint);
    auto stageMask = to_vk_pipeline_stage(stage);

    std::lock_guard lck{m_mutex};
    for (auto &wait : m_wait) {
      if (wait.semaphore == timeline.handle && wait.stageMask == stageMask) {
        wait.value = std::max(wait.value, serial);
        return;
      }
    }
    m_wait.push_back(VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = TimelineImpl::get_timepoint_semaphore(timepoint).handle,
        .value = TimelineImpl::get_timepoint_serial(timepoint),
        .stageMask = stageMask,
        .deviceIndex = 0,
    });
  }

  void submit(span<const CommandBuffer> cmds) noexcept {
    std::lock_guard lck{m_mutex};
    if (m_submissions.size() == MAX_SUBMIT_BATCH_SIZE) {
      submit_all();
    }
    Timepoint timepoint = m_timeline.advance();
    auto &submission = m_submissions.emplace_back(timepoint, m_wait, &m_bump);
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
    // Must copy/retain everything and preferably be noexcept.
    m_gc.retire(timepoint, cmds);
  }

  void present(SwapchainImage image) noexcept {
    std::lock_guard lck{m_mutex};
    auto *img = object_handle_ptr<SwapchainImageImpl>(image);
    auto *gen = object_handle_ptr<SwapchainGenerationImpl>(img->generation);
    auto [presentReady, presentFence] = img->generation.present();
    if (m_submissions.empty()) {
      auto &sub = m_submissions.emplace_back(m_timeline.now(), m_wait, &m_bump);
      sub.signal = presentReady.signal();
      m_wait.clear();
    } else {
      m_submissions.back().signal = presentReady.signal();
    }
    submit_all();
    vulkan::queue_present(
        m_queue, gen->swapchain, img->index,
        {
            .presentReady = presentReady.wait(),
            .presentFence =
                object_handle_ptr<FenceImpl>(presentFence)->node->fence,
        });
  }

private:
  static constexpr size_t MAX_SUBMIT_BATCH_SIZE = 16;

  using bump_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 10>;
  using bump_allocator_ref = AllocatorReference<bump_allocator>;

  struct QueueSubmission {
    explicit QueueSubmission(Timepoint timepoint,
                             span<const VkSemaphoreSubmitInfo> wait,
                             bump_allocator_ref alloc)
        : timepoint(timepoint), wait(wait, alloc), cmds(alloc) {}

    Timepoint timepoint;
    Vector<VkSemaphoreSubmitInfo, bump_allocator_ref> wait;
    Vector<VkCommandBufferSubmitInfo, bump_allocator_ref> cmds;

    vulkan::BinarySemaphore signal;
  };

  void commit(Timepoint timepoint) noexcept {
    if (timepoint.relaxed_poll()) {
      return;
    }
    std::lock_guard lck{m_mutex};
    if (m_submissions.empty()) {
      return;
    }
    if (timepoint < m_submissions.front().timepoint) {
      return;
    }
    submit_all();
  }

  void submit_all() {
    if (m_submissions.empty()) {
      return;
    }
    std::array<VkSubmitInfo2, MAX_SUBMIT_BATCH_SIZE> submitInfos;
    std::array<VkSemaphoreSubmitInfo, MAX_SUBMIT_BATCH_SIZE * 2> signalInfo;
    for (uint32_t i = 0; i < m_submissions.size(); ++i) {
      auto &submission = m_submissions[i];

      uint32_t signalCount = 1;
      signalInfo[2 * i] = VkSemaphoreSubmitInfo{
          .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
          .pNext = nullptr,
          .semaphore =
              TimelineImpl::get_timepoint_semaphore(submission.timepoint)
                  .handle,
          .value = TimelineImpl::get_timepoint_serial(submission.timepoint),
          .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
          .deviceIndex = 0,
      };
      if (submission.signal) {
        signalCount += 1;
        signalInfo[2 * i + 1] = VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = submission.signal.handle,
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .deviceIndex = 0,
        };
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
          .signalSemaphoreInfoCount = signalCount,
          .pSignalSemaphoreInfos = signalInfo.data() + 2 * i,
      };
    }
    {
      ZoneScopedN("vkQueueSubmit2");
      VkResult result = vkQueueSubmit2(m_queue.handle, m_submissions.size(),
                                       submitInfos.data(), VK_NULL_HANDLE);
      if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit to queue");
      }
    }
    m_submissions.clear();
    m_bump.release();
  }
  Timeline m_timeline;
  GarbageCollector m_gc;
  vulkan::Queue m_queue;
  QueueFlags m_flags;
  bump_allocator m_bump;
  Vector<VkSemaphoreSubmitInfo> m_wait;
  SmallVector<QueueSubmission, MAX_SUBMIT_BATCH_SIZE, strobe::NullAllocator>
      m_submissions;
  std::mutex m_mutex;
};

} // namespace strobe::rhi
