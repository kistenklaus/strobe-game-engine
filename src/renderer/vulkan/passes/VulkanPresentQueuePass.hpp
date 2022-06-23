#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanPresentQueuePass : public RenderPass {
 public:
  VulkanPresentQueuePass(VulkanRendererBackend* renderer);

  void execute() override;

  u32 getWaitSemaphoresSink() { return m_waitSemsSink; }
  u32 getQueueSink() { return m_queueSink; }

 private:
  const u32 m_queueSink;
  const u32 m_waitSemsSink;
};

}  // namespace sge::vulkan
