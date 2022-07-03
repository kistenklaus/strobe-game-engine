#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanResetCmdPoolPass : public RenderPass {
 public:
  VulkanResetCmdPoolPass(VulkanRendererBackend* renderer,
                         const std::string name);
  void execute() override;

  u32 getCommandPoolSink() const { return m_poolSink; }
  u32 getCommandPoolSource() const { return m_poolSource; }

 private:
  const u32 m_poolSink;
  const u32 m_poolSource;
};

}  // namespace sge::vulkan
