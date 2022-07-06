#pragma once
#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VulkanAcquireSwapchainFrame : public RenderPass {
 public:
  explicit VulkanAcquireSwapchainFrame(VRendererBackend* renderer,
                                       const std::string name);
  void execute() override;

  // Getters:
  u32 getSignalSemaphoreSource() const { return m_singalSemaphoresSource; }
  u32 getSwapchainImageView() const { return m_swapchainImageViewSource; }

 private:
  const u32 m_singalSemaphoresSource;
  const u32 m_swapchainImageViewSource;
  const u32 m_swapchainFrameIndexSource;

  std::vector<u32> m_signalSemphores;
  u32 m_imageView;
  u32 m_frameIndex;
};

}  // namespace sge::vulkan
