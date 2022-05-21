#include "strb/apis/vulkan/Queue.hpp"

namespace strb::vulkan {

void Queue::submit(const CommandBuffer& commandBuffer,
                   const strb::vector<Semaphore>& waitSemaphores,
                   const strb::vector<Semaphore>& signalSemaphores) {
  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;

  // OVERHEAD convert to v<VkSemaphore>
  strb::vector<VkSemaphore> vkWaitSems(waitSemaphores.begin(),
                                       waitSemaphores.end());
  submitInfo.waitSemaphoreCount = vkWaitSems.size();
  submitInfo.pWaitSemaphores = vkWaitSems.data();
  VkPipelineStageFlags waitStageMasks = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask = &waitStageMasks;
  strb::vector<VkCommandBuffer> buffers = {commandBuffer};
  submitInfo.commandBufferCount = buffers.size();
  submitInfo.pCommandBuffers = buffers.data();
  strb::vector<VkSemaphore> vkSignalSems(signalSemaphores.begin(),
                                         signalSemaphores.end());
  submitInfo.signalSemaphoreCount = vkSignalSems.size();
  submitInfo.pSignalSemaphores = vkSignalSems.data();
  // TODO implement passing of a Fence object.
  const VkResult result = vkQueueSubmit(this->queue, 1, &submitInfo, nullptr);
  ASSERT_VKRESULT(result);
}

void Queue::present(const Swapchain& swapchain, const uint32_t frameIndex,
                    const strb::vector<Semaphore>& waitSemaphores) {
  VkPresentInfoKHR presentInfo;
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  strb::vector<VkSemaphore> vkSems(waitSemaphores.begin(),
                                   waitSemaphores.end());
  presentInfo.waitSemaphoreCount = vkSems.size();
  presentInfo.pWaitSemaphores = vkSems.data();
  presentInfo.swapchainCount = 1;
  VkSwapchainKHR vkSwapchain = swapchain;
  presentInfo.pSwapchains = &vkSwapchain;
  presentInfo.pImageIndices = &frameIndex;
  presentInfo.pResults = nullptr;
  const VkResult result = vkQueuePresentKHR(queue, &presentInfo);
  ASSERT_VKRESULT(result);
}

}  // namespace strb::vulkan
