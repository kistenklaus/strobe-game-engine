#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VCmdPoolMultiSrcPass : public RenderPass {
 public:
  explicit VCmdPoolMultiSrcPass(VRendererBackend* renderer,
                                const std::string name,
                                const QueueFamilyType queueFamily,
                                const u32 count);
  void dispose() override;

 private:
  const std::vector<command_pool> m_pools;
  const u32 m_poolsSource;
};

}  // namespace sge::vulkan
