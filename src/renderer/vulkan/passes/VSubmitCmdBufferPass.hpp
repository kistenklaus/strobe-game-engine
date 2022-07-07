#pragma once
#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VSubmitCmdBufferPass : public RenderPass {
 public:
  explicit VSubmitCmdBufferPass(VRendererBackend* renderer,
                                const std::string name);

  void execute() override;

 private:
  const u32 m_fenceSink;
  const u32 m_queueSink;
  const u32 m_commandBuffersSink;
  const u32 m_waitSemaphoresSink;
  const u32 m_signalSemaphoresSource;
  std::vector<semaphore> m_signalSemaphores;
};
}  // namespace sge::vulkan
