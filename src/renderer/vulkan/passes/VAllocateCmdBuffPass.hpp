#pragma once

#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VAllocateCmdBuffPass : public RenderPass {
 public:
  explicit VAllocateCmdBuffPass(VRendererBackend* renderer,
                                const std::string name);
  void execute() override;

 private:
  sink<command_pool> m_poolSink;
  source<command_buffer> m_bufferSource;

  command_buffer m_buffer;
};

}  // namespace sge::vulkan
