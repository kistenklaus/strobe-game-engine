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
  sink<fence> m_fenceSink;
  sink<queue> m_queueSink;
  sink<command_buffer> m_commandBuffersSink;
  sink<std::vector<semaphore>> m_waitSemaphoresSink;
  source<std::vector<semaphore>> m_signalSemaphoresSource;

  std::vector<semaphore> m_signalSemaphores;
};
}  // namespace sge::vulkan
