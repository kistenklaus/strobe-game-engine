#include "strobe/rhi/vulkan/queue.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

void queue_submit(Queue queue, const SubmitInfo &info) {
  assert(queue);

  static constexpr size_t SCRATCH_SIZE =
      sizeof(VkSemaphoreSubmitInfo) * 8 + sizeof(VkCommandBufferSubmitInfo) * 2;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};

  const size_t waitSemCount =
      info.waitTimelineSemaphores.size() + info.waitBinarySemaphores.size();
  Vector<VkSemaphoreSubmitInfo, scratch_allocator_ref> waitSemSubmitInfos{
      waitSemCount, &scratch};
  size_t i = 0;
  for (size_t j = 0; j < info.waitTimelineSemaphores.size(); ++j, ++i) {
    waitSemSubmitInfos[i] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = info.waitTimelineSemaphores[j].semaphore.handle,
        .value = info.waitTimelineSemaphores[j].value,
        .stageMask = info.waitTimelineSemaphores[j].stage,
        .deviceIndex = 0,
    };
  }
  for (size_t j = 0; j < info.waitBinarySemaphores.size(); ++j, ++i) {
    waitSemSubmitInfos[i] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = info.waitBinarySemaphores[j].semaphore.handle,
        .value = 0,
        .stageMask = info.waitBinarySemaphores[j].stage,
        .deviceIndex = 0,
    };
  }

  const size_t cmdCount = info.command_buffers.size();
  Vector<VkCommandBufferSubmitInfo, scratch_allocator_ref> cmdSubmitInfos{
      cmdCount, &scratch};
  for (size_t j = 0; j < cmdCount; ++j) {
    cmdSubmitInfos[j] = VkCommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = info.command_buffers[j].handle,
        .deviceMask = 0,
    };
  }

  const size_t signalSemCount =
      info.signalTimelineSemaphores.size() + info.signalBinarySemaphores.size();
  Vector<VkSemaphoreSubmitInfo, scratch_allocator_ref> signalSemSubmitInfos{
      signalSemCount, &scratch};
  i = 0;
  for (size_t j = 0; j < info.signalTimelineSemaphores.size(); ++j, ++i) {
    signalSemSubmitInfos[i] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = info.signalTimelineSemaphores[j].semaphore.handle,
        .value = info.signalTimelineSemaphores[j].value,
        .stageMask = info.signalTimelineSemaphores[j].stage,
        .deviceIndex = 0,
    };
  }
  for (size_t j = 0; j < info.signalBinarySemaphores.size(); ++j, ++i) {
    signalSemSubmitInfos[i] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = info.signalBinarySemaphores[j].semaphore.handle,
        .value = 0,
        .stageMask = info.signalBinarySemaphores[j].stage,
        .deviceIndex = 0,
    };
  }

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
  {
    ZoneScopedN("vkQueueSubmit2");
    VkResult result =
        vkQueueSubmit2(queue.handle, 1, &submitInfo, info.fence.handle);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to submit to queue");
    }
  }
}
void wait_queue_idle(Queue queue) {
  assert(queue);
  {
    ZoneScopedN("vkQueueWaitIdle");
    VkResult result = vkQueueWaitIdle(queue.handle);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Wait to wait for queue idle");
    }
  }
}
PresentStatus queue_present(Queue queue, Swapchain swapchain,
                            uint32_t imageIndex, const PresentInfo &info) {
  assert(queue);
  assert(swapchain);

  static constexpr size_t SCRATCH_SIZE = sizeof(VkSemaphore) * 8;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};

  Vector<VkSemaphore, scratch_allocator_ref> sems{
      info.waitBinarySemaphores.size(), &scratch};
  for (size_t i = 0; i < info.waitBinarySemaphores.size(); ++i) {
    sems[i] = info.waitBinarySemaphores[i].handle;
  }

  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = static_cast<uint32_t>(sems.size()),
      .pWaitSemaphores = sems.data(),
      .swapchainCount = 1,
      .pSwapchains = &swapchain.handle,
      .pImageIndices = &imageIndex,
      .pResults = nullptr,
  };
  VkResult result;
  {
    ZoneScopedN("vkQueuePresentKHR");
    result = vkQueuePresentKHR(queue.handle, &presentInfo);
  }
  if (result == VK_SUCCESS) {
    return PresentStatus::success;
  } else if (result == VK_SUBOPTIMAL_KHR) {
    return PresentStatus::suboptimal;
  } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return PresentStatus::out_of_date;
  } else {
    throw std::runtime_error("Failed to present to queue");
  }
}

} // namespace strobe::rhi::vulkan
