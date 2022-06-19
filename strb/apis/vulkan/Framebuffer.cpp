#include "strb/apis/vulkan/Framebuffer.hpp"

#include <cassert>

#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

Framebuffer::Framebuffer(const Device& device, const RenderPass& renderPass,
                         const uint32_t width, const uint32_t height,
                         strb::vector<ImageView>& attachments)
    : device(&device),
      renderPass(&renderPass),
      width(width),
      height(height),
      attachments(attachments) {
  VkFramebufferCreateInfo framebufferCreateInfo;
  framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferCreateInfo.pNext = nullptr;
  framebufferCreateInfo.flags = 0;
  framebufferCreateInfo.renderPass = renderPass;
  strb::vector<VkImageView> attachmentsImageView(attachments.size());
  for (uint64_t i = 0; i < attachments.size(); i++) {
    attachmentsImageView[i] = attachments[i];
  }
  framebufferCreateInfo.attachmentCount = attachmentsImageView.size();
  framebufferCreateInfo.pAttachments = attachmentsImageView.data();
  framebufferCreateInfo.width = width;
  framebufferCreateInfo.height = height;
  framebufferCreateInfo.layers = 1;
  VkResult result = vkCreateFramebuffer(*this->device, &framebufferCreateInfo,
                                        nullptr, &this->framebuffer);
  ASSERT_VKRESULT(result);
  assert(this->device != nullptr);
  assert(this->framebuffer != VK_NULL_HANDLE);
};

void Framebuffer::destroy() {
  assert(this->device != nullptr);
  assert(this->framebuffer != VK_NULL_HANDLE);
  vkDestroyFramebuffer(*this->device, this->framebuffer, nullptr);
  dexec(this->device = nullptr);
  dexec(this->framebuffer = VK_NULL_HANDLE);
}

void Framebuffer::recreate(const uint32_t width, const uint32_t height,
                           const strb::vector<ImageView>& attachments) {
  vkDestroyFramebuffer(*this->device, this->framebuffer, nullptr);
  this->width = width;
  this->height = height;
  this->attachments = attachments;

  VkFramebufferCreateInfo framebufferCreateInfo;
  framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferCreateInfo.pNext = nullptr;
  framebufferCreateInfo.flags = 0;
  framebufferCreateInfo.renderPass = *this->renderPass;
  strb::vector<VkImageView> attachmentsImageView(attachments.size());
  for (uint64_t i = 0; i < attachments.size(); i++) {
    attachmentsImageView[i] = attachments[i];
  }
  framebufferCreateInfo.attachmentCount = attachmentsImageView.size();
  framebufferCreateInfo.pAttachments = attachmentsImageView.data();
  framebufferCreateInfo.width = width;
  framebufferCreateInfo.height = height;
  framebufferCreateInfo.layers = 1;
  VkResult result = vkCreateFramebuffer(*this->device, &framebufferCreateInfo,
                                        nullptr, &this->framebuffer);
  ASSERT_VKRESULT(result);
  assert(this->device != nullptr);
  assert(this->framebuffer != VK_NULL_HANDLE);
}

}  // namespace strb::vulkan
