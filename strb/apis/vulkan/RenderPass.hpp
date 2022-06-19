#pragma once
#include "strb/apis/vulkan/CommandBuffer.hpp"
#include "strb/apis/vulkan/CommandPool.hpp"
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/Format.hpp"
#include "strb/apis/vulkan/Framebuffer.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct Framebuffer;

struct RenderPass {
 private:
  const Device* device;
  VkRenderPass renderPass;
  Format format;

 public:
  inline RenderPass() : device(nullptr), renderPass(VK_NULL_HANDLE) {}
  RenderPass(const Device& device, const Format format);
  void destroy();
  operator VkRenderPass() const { return renderPass; }
  void begin(const CommandBuffer& commandBuffer, const Framebuffer& framebuffer,
             const int32_t viewportX, const int32_t viewportY,
             const uint32_t viewportWidth, const uint32_t viewportHeight);
  void end(const CommandBuffer& commandBuffer);
  void recreate();
};

}  // namespace strb::vulkan
