#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VulkanPresentQueuePass : public RenderPass {
 public:
  explicit VulkanPresentQueuePass(VRendererBackend* renderer,
                                  const std::string name);

  void execute() override;

 private:
  const u32 m_queueSink;
  const u32 m_waitSemsSink;
};

}  // namespace sge::vulkan
