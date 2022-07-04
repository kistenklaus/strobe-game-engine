#pragma once
#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanSubmitCmdBufferPass : public RenderPass {
 public:
  explicit VulkanSubmitCmdBufferPass(VulkanRendererBackend* renderer,
                                     const std::string name);

  void execute() override;

 private:
  const u32 m_fenceSink;
  const u32 m_queueSink;
  const u32 m_commandBuffersSink;
  const u32 m_waitSemaphoresSink;
  const u32 m_signalSemaphoresSource;
  std::vector<u32> m_signalSemaphores;
};
}  // namespace sge::vulkan
