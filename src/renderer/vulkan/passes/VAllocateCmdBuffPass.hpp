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

  u32 getPoolSink() const { return m_poolSink; }
  u32 getCmdBufferSource() const { return m_bufferSource; }

 private:
  const u32 m_poolSink;
  const u32 m_bufferSource;

  command_buffer m_buffer;
};

}  // namespace sge::vulkan
