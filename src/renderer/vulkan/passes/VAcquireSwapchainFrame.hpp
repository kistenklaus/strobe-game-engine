#pragma once
#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VAcquireSwapchainFrame : public RenderPass {
 public:
  explicit VAcquireSwapchainFrame(VRendererBackend* renderer,
                                  const std::string name);
  void execute() override;

 private:
  source<std::vector<semaphore>> m_singalSemaphoresSource;
  source<imageview> m_swapchainImageViewSource;
  source<u32> m_swapchainFrameIndexSource;

  std::vector<semaphore> m_signalSemphores;
  imageview m_imageView;
  u32 m_frameIndex;
};

}  // namespace sge::vulkan
