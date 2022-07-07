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
  sink<fence> m_fenceSink;
  sink<command_pool> m_poolSink;
  source<fence> m_fenceSource;
  source<command_pool> m_poolSource;
  std::set<fence> m_seenFences;
};

}  // namespace sge::vulkan
