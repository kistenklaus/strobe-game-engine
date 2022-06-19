#include "strb/apis/vulkan/RenderPass.hpp"

#include <cassert>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

RenderPass::RenderPass(const Device& device, const Format format)
    : device(&device), format(format) {
  VkAttachmentDescription attachmentDescription;
  attachmentDescription.flags = 0;
  attachmentDescription.format = format::toVkFormat(format);
  attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  VkAttachmentReference attachmentReference;
  attachmentReference.attachment = 0;
  attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkSubpassDescription subpassDescription;
  subpassDescription.flags = 0;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = nullptr;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &attachmentReference;
  subpassDescription.pResolveAttachments = nullptr;
  subpassDescription.pDepthStencilAttachment = nullptr;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = nullptr;
  VkSubpassDependency subpassDependency;
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependency.dependencyFlags = 0;
  VkRenderPassCreateInfo renderPassCreateInfo;
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.pNext = nullptr;
  renderPassCreateInfo.flags = 0;
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &attachmentDescription;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpassDescription;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &subpassDependency;
  VkResult result = vkCreateRenderPass(*this->device, &renderPassCreateInfo,
                                       nullptr, &this->renderPass);
  ASSERT_VKRESULT(result);
}

void RenderPass::destroy() {
  assert(device != nullptr);
  assert(this->renderPass != VK_NULL_HANDLE);
  vkDestroyRenderPass(*device, this->renderPass, nullptr);
  dexec(device = nullptr);
  dexec(this->renderPass = VK_NULL_HANDLE);
}

void RenderPass::begin(const CommandBuffer& commandBuffer,
                       const Framebuffer& framebuffer, const int32_t viewportX,
                       const int32_t viewportY, const uint32_t viewportWidth,
                       const uint32_t viewportHeight) {
  assert(this->device != nullptr);
  assert(this->renderPass != VK_NULL_HANDLE);

  VkRenderPassBeginInfo renderPassBeginInfo;
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.pNext = nullptr;
  renderPassBeginInfo.renderPass = renderPass;
  renderPassBeginInfo.framebuffer = framebuffer;
  // TODO: pls fix magic
  renderPassBeginInfo.renderArea.offset = {viewportX, viewportY};
  renderPassBeginInfo.renderArea.extent = {viewportWidth, viewportHeight};

  VkClearValue clearValue;
  clearValue.color.float32[0] = 0;
  clearValue.color.float32[1] = 0;
  clearValue.color.float32[2] = 0;
  clearValue.color.float32[3] = 1;

  renderPassBeginInfo.clearValueCount = 1;
  renderPassBeginInfo.pClearValues = &clearValue;

  vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(const CommandBuffer& commandBuffer) {
  assert(device != nullptr);
  assert(renderPass != VK_NULL_HANDLE);
  vkCmdEndRenderPass(commandBuffer);
}

void RenderPass::recreate() {
  vkDestroyRenderPass(*this->device, this->renderPass, nullptr);
  VkAttachmentDescription attachmentDescription;
  attachmentDescription.flags = 0;
  attachmentDescription.format = format::toVkFormat(format);
  attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  VkAttachmentReference attachmentReference;
  attachmentReference.attachment = 0;
  attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkSubpassDescription subpassDescription;
  subpassDescription.flags = 0;
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = nullptr;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &attachmentReference;
  subpassDescription.pResolveAttachments = nullptr;
  subpassDescription.pDepthStencilAttachment = nullptr;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = nullptr;
  VkSubpassDependency subpassDependency;
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependency.dependencyFlags = 0;
  VkRenderPassCreateInfo renderPassCreateInfo;
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.pNext = nullptr;
  renderPassCreateInfo.flags = 0;
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &attachmentDescription;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpassDescription;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &subpassDependency;
  VkResult result = vkCreateRenderPass(*this->device, &renderPassCreateInfo,
                                       nullptr, &this->renderPass);
  ASSERT_VKRESULT(result);
}

}  // namespace strb::vulkan
