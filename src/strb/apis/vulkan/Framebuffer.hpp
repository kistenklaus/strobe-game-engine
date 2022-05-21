#pragma once
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/ImageView.hpp"
#include "strb/apis/vulkan/RenderPass.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct RenderPass;

struct Framebuffer {
 private:
  const Device* device;
  const RenderPass* renderPass;
  VkFramebuffer framebuffer;
  uint32_t width;
  uint32_t height;
  strb::vector<ImageView> attachments;

 public:
  inline Framebuffer()
      : device(nullptr), renderPass(nullptr), framebuffer(VK_NULL_HANDLE) {}
  Framebuffer(const Device& device, const RenderPass& renderPass,
              const uint32_t width, const uint32_t height,
              strb::vector<ImageView>& attachments);
  void destroy();
  operator VkFramebuffer() const { return this->framebuffer; }
  void recreate(const uint32_t width, const uint32_t height,
                const strb::vector<ImageView>& attachments);
};

}  // namespace strb::vulkan
