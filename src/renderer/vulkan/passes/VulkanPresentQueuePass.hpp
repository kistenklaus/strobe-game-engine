#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanPresentQueuePass : public RenderPass {
 public:
  explicit VulkanPresentQueuePass(VulkanRendererBackend* renderer,
                                  const std::string name);

  void execute() override;

 private:
  const u32 m_queueSink;
  const u32 m_waitSemsSink;
};

}  // namespace sge::vulkan
