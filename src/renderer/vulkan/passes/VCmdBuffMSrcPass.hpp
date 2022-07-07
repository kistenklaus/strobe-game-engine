#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VCmdBuffMSrcPass : public RenderPass {
 public:
  VCmdBuffMSrcPass(VRendererBackend* renderer, const std::string name,
                   const u32 count);

  void create() override;
  void dispose() override;

 private:
  const u32 m_poolSink;
  const u32 m_cmdbufferSrc;
  const u32 m_count;
  std::vector<command_buffer> m_buffers;
};

}  // namespace sge::vulkan
