#pragma once

#include <set>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VResetCmdBuffPass : public RenderPass {
 public:
  explicit VResetCmdBuffPass(VRendererBackend* renderer,
                             const std::string name);
  void recreate() override;

  void execute() override;

 private:
  sinksource<command_buffer> m_cmdbufferSS;
  sinksource<fence> m_fenceSS;

  std::set<fence> m_seenFences;
};

}  // namespace sge::vulkan
