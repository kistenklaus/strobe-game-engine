#include "strobe/gpu/device/queue.hpp"
#include "strobe/gpu/device/binary_semaphore_impl.hpp"
#include "strobe/gpu/device/command_buffer_handle_alloc.hpp"
#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/fence_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/queue_impl.hpp"
#include "strobe/gpu/device/queue_submission.hpp"
#include "strobe/gpu/device/swapchain_generation_impl.hpp"
#include "strobe/gpu/device/timeline_semaphore_impl.hpp"
#include <mutex>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

Queue::Queue(const Queue &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<QueueImpl>(m_handle);
  }
}

Queue::Queue(Queue &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Queue &Queue::operator=(const Queue &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<QueueImpl>(o.m_handle);
  }
  unpin_void_handle<QueueImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}
Queue &Queue::operator=(Queue &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<QueueImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Queue::~Queue() noexcept { unpin_void_handle<QueueImpl>(m_handle); }

void Queue::submit(const SubmitInfo &info) {
  ZoneScopedN("Queue::submit");
  auto *impl = void_handle_ptr<QueueImpl>(m_handle);

  QueueSubmission *submission = impl->native->acquire_submission();

  //
  // Keep all user-provided command buffers alive until this submission
  // completes.
  //
  submission->commandBuffers.assign(info.cmds.begin(), info.cmds.end());

  submission->binarySemaphores.resize(info.wait.size() + info.signal.size());

  submission->timelineSemaphores.resize(info.waitTimeline.size() +
                                        info.signalTimeline.size());

  submission->fence = info.fence;

  static constexpr size_t SCRATCH_SIZE =
      sizeof(VkSemaphoreSubmitInfo) * 8 + sizeof(VkCommandBufferSubmitInfo) * 4;

  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};

  const size_t waitSemCount = info.waitTimeline.size() + info.wait.size();

  const size_t signalSemCount = info.signalTimeline.size() + info.signal.size();

  //
  // In the common case we need:
  //
  //   post-acquire transition
  //   user command buffers...
  //   pre-present transition
  //
  // Reserve two additional entries. Submitting multiple swapchain images in
  // one submission is allowed to fall back to another allocation.
  //
  Vector<VkCommandBufferSubmitInfo, scratch_allocator_ref> cmdSubmitInfos{
      0, &scratch};
  cmdSubmitInfos.reserve(info.cmds.size() + 2);

  Vector<VkSemaphoreSubmitInfo, scratch_allocator_ref> waitSemSubmitInfos{
      waitSemCount, &scratch};

  Vector<VkSemaphoreSubmitInfo, scratch_allocator_ref> signalSemSubmitInfos{
      signalSemCount + 1, &scratch};

  //
  // Wait timeline semaphores.
  //
  size_t waitSemIndex = 0;

  for (size_t j = 0; j < info.waitTimeline.size(); ++j) {
    const auto &wait = info.waitTimeline[j];

    submission->timelineSemaphores[j] = wait.semaphore;

    auto *semImpl =
        void_handle_ptr<TimelineSemaphoreImpl>(wait.semaphore.m_handle);

    waitSemSubmitInfos[waitSemIndex++] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semImpl->semaphore.handle,
        .value = wait.value,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .deviceIndex = 0,
    };
  }

  //
  // Wait binary semaphores.
  //
  // If a binary semaphore carries swapchain metadata and is used as a wait,
  // it represents the semaphore supplied to Swapchain::acquire().
  //
  // Therefore insert:
  //
  //     UNDEFINED -> GENERAL
  //
  // before all user command buffers.
  //
  for (size_t j = 0; j < info.wait.size(); ++j) {
    const auto &wait = info.wait[j];
    const BinarySemaphore &semaphore = wait.semaphore;

    //
    // Take the QueueSubmission strong reference BEFORE potentially dropping
    // SwapchainImageState::acquireSignal below.
    //
    submission->binarySemaphores[j] = semaphore;

    auto *semImpl = void_handle_ptr<BinarySemaphoreImpl>(semaphore.m_handle);

    waitSemSubmitInfos[waitSemIndex++] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semImpl->semaphore.handle,
        .value = 0,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .deviceIndex = 0,
    };

    SwapchainGenerationImpl *genImpl = semImpl->swapchainGeneration;
    if (genImpl == nullptr) {
      continue;
    }

    [[maybe_unused]] bool found = false;

    for (uint32_t imageIndex = 0;
         imageIndex < static_cast<uint32_t>(genImpl->images.size());
         ++imageIndex) {
      auto &state = genImpl->images[imageIndex];

      if (!state.acquireSignal ||
          state.acquireSignal.m_handle != semaphore.m_handle) {
        continue;
      }

      vulkan::CommandBuffer transition =
          genImpl->getPostAcquireCmdBuf(imageIndex, impl->native->queue.family);

      cmdSubmitInfos.push_back(VkCommandBufferSubmitInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
          .pNext = nullptr,
          .commandBuffer = transition.handle,
          .deviceMask = 0,
      });

      //
      // Consume the one-shot acquire metadata.
      //
      // The semaphore cannot die here because QueueSubmission already owns
      // a strong reference to it.
      //
      semImpl->swapchainGeneration = nullptr;
      state.acquireSignal = {};

      found = true;
      break;
    }

    //
    // A tagged semaphore used as a wait must correspond to an outstanding
    // acquire operation.
    //
    assert(found);
  }

  //
  // User command buffers.
  //
  // These must come after all post-acquire transitions.
  //
  for (size_t j = 0; j < info.cmds.size(); ++j) {
    auto *cmdImpl = void_handle_ptr<CommandBufferImpl,
                                    strobe::gpu::cmd_buf_handle_allocator_ref>(
        info.cmds[j].m_handle);

    cmdSubmitInfos.push_back(VkCommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmdImpl->cmd.handle,
        .deviceMask = 0,
    });
  }

  //
  // Signal timeline semaphores.
  //
  // Slot zero is reserved for the queue's internal timeline semaphore.
  //
  size_t signalSemIndex = 1;

  for (size_t j = 0; j < info.signalTimeline.size(); ++j) {
    const auto &signal = info.signalTimeline[j];

    submission->timelineSemaphores[info.waitTimeline.size() + j] =
        signal.semaphore;

    auto *semImpl =
        void_handle_ptr<TimelineSemaphoreImpl>(signal.semaphore.m_handle);

    signalSemSubmitInfos[signalSemIndex++] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semImpl->semaphore.handle,
        .value = signal.value,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .deviceIndex = 0,
    };
  }

  //
  // Signal binary semaphores.
  //
  // If a binary semaphore carries swapchain metadata and is used as a signal,
  // it represents SwapchainImage::presentReady().
  //
  // Therefore append:
  //
  //     GENERAL -> PRESENT_SRC_KHR
  //
  // after all user command buffers and before the semaphore is signaled.
  //
  for (size_t j = 0; j < info.signal.size(); ++j) {
    const auto &signal = info.signal[j];
    const BinarySemaphore &semaphore = signal.semaphore;

    submission->binarySemaphores[info.wait.size() + j] = semaphore;

    auto *semImpl = void_handle_ptr<BinarySemaphoreImpl>(semaphore.m_handle);

    signalSemSubmitInfos[signalSemIndex++] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semImpl->semaphore.handle,
        .value = 0,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .deviceIndex = 0,
    };

    SwapchainGenerationImpl *genImpl = semImpl->swapchainGeneration;
    if (genImpl == nullptr) {
      continue;
    }

    [[maybe_unused]] bool found = false;

    for (uint32_t imageIndex = 0;
         imageIndex < static_cast<uint32_t>(genImpl->images.size());
         ++imageIndex) {
      auto &state = genImpl->images[imageIndex];

      if (!state.presentReady ||
          state.presentReady.m_handle != semaphore.m_handle) {
        continue;
      }

      vulkan::CommandBuffer transition =
          genImpl->getPrePresentCmdBuf(imageIndex, impl->native->queue.family);

      cmdSubmitInfos.push_back(VkCommandBufferSubmitInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
          .pNext = nullptr,
          .commandBuffer = transition.handle,
          .deviceMask = 0,
      });

      found = true;
      break;
    }

    //
    // A tagged semaphore used as a signal must be one of this generation's
    // presentReady semaphores.
    //
    assert(found);
  }

  assert(waitSemIndex == waitSemSubmitInfos.size());
  assert(signalSemIndex == signalSemSubmitInfos.size());

  VkSubmitInfo2 submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .pNext = nullptr,
      .flags = 0,
      .waitSemaphoreInfoCount =
          static_cast<uint32_t>(waitSemSubmitInfos.size()),
      .pWaitSemaphoreInfos = waitSemSubmitInfos.data(),
      .commandBufferInfoCount = static_cast<uint32_t>(cmdSubmitInfos.size()),
      .pCommandBufferInfos = cmdSubmitInfos.data(),
      .signalSemaphoreInfoCount =
          static_cast<uint32_t>(signalSemSubmitInfos.size()),
      .pSignalSemaphoreInfos = signalSemSubmitInfos.data(),
  };

  VkFence vkFence =
      info.fence ? void_handle_ptr<FenceImpl>(info.fence.m_handle)->fence.handle
                 : VK_NULL_HANDLE;

  {
    std::lock_guard lck{impl->native->mutex};

    submission->timelineValue = impl->native->nextTimelineValue++;

    signalSemSubmitInfos[0] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = impl->native->timeline.handle,
        .value = submission->timelineValue,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .deviceIndex = 0,
    };
    VkResult result;
    {
      ZoneScopedN("vkQueueSubmit2");
      result =
          vkQueueSubmit2(impl->native->queue.handle, 1, &submitInfo, vkFence);
    }
    if (result != VK_SUCCESS) {
      impl->native->release_submission(submission);
      throw std::runtime_error("Failed to submit to queue");
    }

    impl->native->commit_submission(submission);
  }
}

bool Queue::present(SwapchainImage &&image) {
  ZoneScopedN("Queue::present");
  auto *impl = void_handle_ptr<QueueImpl>(m_handle);
  auto *generation = void_handle_ptr<SwapchainGenerationImpl>(image.m_handle);
  auto &image_state = generation->images[image.m_index];

  auto *semaphoreImpl =
      void_handle_ptr<BinarySemaphoreImpl>(image_state.presentReady.m_handle);
  const VkSemaphore presentReady = semaphoreImpl->semaphore.handle;

  if (image_state.pending) {
    image_state.fence.wait();
    image_state.pending = false;
  }
  image_state.fence.reset();
  auto *fence_impl = void_handle_ptr<FenceImpl>(image_state.fence.m_handle);

  VkSwapchainPresentFenceInfoKHR fenceInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR,
      .pNext = nullptr,
      .swapchainCount = 1,
      .pFences = &fence_impl->fence.handle,
  };

  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = &fenceInfo,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &presentReady,
      .swapchainCount = 1,
      .pSwapchains = &generation->swapchain.handle,
      .pImageIndices = &image.m_index,
      .pResults = nullptr,
  };
  {
    std::lock_guard lock{impl->native->mutex};
    VkResult result;
    {
      ZoneScopedN("vkQueuePresentKHR");
      result = vkQueuePresentKHR(impl->native->queue.handle, &presentInfo);
    }
    switch (result) {
    case VK_SUCCESS:
      image_state.pending = true;
      return true;
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
      image_state.pending = true;
      return false;
    case VK_ERROR_SURFACE_LOST_KHR:
      image_state.pending = true;
      throw std::runtime_error{"Surface lost during presentation"};
    default:
      throw std::runtime_error{"Failed to present to queue"};
    }
  }
}

} // namespace strobe::gpu
