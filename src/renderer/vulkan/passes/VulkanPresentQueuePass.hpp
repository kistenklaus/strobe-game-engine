#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanPresentQueuePass : public RenderPass {
 public:
  VulkanPresentQueuePass(VulkanRendererBackend* renderer,
                         const std::string name);

  void execute() override;

  u32 getWaitSemaphoresSink() const { return m_waitSemsSink; }
  u32 getQueueSink() const { return m_queueSink; }

 private:
  const u32 m_queueSink;
  const u32 m_waitSemsSink;
};

}  // namespace sge::vulkan
