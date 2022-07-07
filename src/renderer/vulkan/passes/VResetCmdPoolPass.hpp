#pragma once

#include <set>
#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VResetCmdPoolPass : public RenderPass {
 public:
  explicit VResetCmdPoolPass(VRendererBackend* renderer,
                             const std::string name);
  void execute() override;
  void recreate() override;

 private:
  const u32 m_fenceSink;
  const u32 m_poolSink;
  const u32 m_fenceSource;
  const u32 m_poolSource;
  std::set<fence> m_seenFences;
};

}  // namespace sge::vulkan
