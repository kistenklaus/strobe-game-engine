#pragma once

#include <set>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VulkanResetCmdBuffPass : public RenderPass {
 public:
  explicit VulkanResetCmdBuffPass(VRendererBackend* renderer,
                                  const std::string name);
  void recreate() override;

  void execute() override;

 private:
  const u32 m_cmdbufferSink;
  const u32 m_fenceSink;
  const u32 m_cmdbufferSrc;
  const u32 m_fenceSrc;

  std::set<u32> m_seenFences;
};

}  // namespace sge::vulkan
