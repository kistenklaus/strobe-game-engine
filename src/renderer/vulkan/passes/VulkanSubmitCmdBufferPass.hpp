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

  u32 getQueueSink() const { return m_queueSink; }
  u32 getCommandBuffersSink() const { return m_commandBuffersSink; }
  u32 getWaitSemaphoresSink() const { return m_waitSemaphoresSink; }
  u32 getSignalSemaphoreSource() const { return m_signalSemaphoresSource; }

 private:
  const u32 m_queueSink;
  const u32 m_commandBuffersSink;
  const u32 m_waitSemaphoresSink;
  const u32 m_signalSemaphoresSource;
  std::vector<u32> m_signalSemaphores;
};
}  // namespace sge::vulkan
