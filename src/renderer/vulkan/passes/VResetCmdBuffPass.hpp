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
  sink<command_buffer> m_cmdbufferSink;
  sink<fence> m_fenceSink;
  source<command_buffer> m_cmdbufferSrc;
  source<fence> m_fenceSrc;

  std::set<fence> m_seenFences;
};

}  // namespace sge::vulkan
