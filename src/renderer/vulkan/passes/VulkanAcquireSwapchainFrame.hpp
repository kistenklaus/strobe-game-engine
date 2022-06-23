#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanAcquireSwapchainFrame : public RenderPass {
 public:
  explicit VulkanAcquireSwapchainFrame(VulkanRendererBackend* renderer);
  void execute() override;

  // Getters:
  u32 getSignalSemaphoreSource() { return m_singalSemSrc; }

 private:
  const u32 m_signalSem;
  const u32 m_singalSemSrc;
};

}  // namespace sge::vulkan
