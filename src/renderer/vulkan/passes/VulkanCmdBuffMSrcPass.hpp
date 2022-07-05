#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

class VulkanCmdBuffMSrcPass : public RenderPass {
 public:
  VulkanCmdBuffMSrcPass(VulkanRendererBackend* renderer, const std::string name,
                        const u32 count);

  void create() override;
  void dispose() override;

 private:
  const u32 m_poolSink;
  const u32 m_cmdbufferSrc;
  const u32 m_count;
  std::vector<u32> m_buffers;
};

}  // namespace sge::vulkan
