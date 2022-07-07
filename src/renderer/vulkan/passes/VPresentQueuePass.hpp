#pragma once

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

class VPresentQueuePass : public RenderPass {
 public:
  explicit VPresentQueuePass(VRendererBackend* renderer,
                             const std::string name);

  void execute() override;

 private:
  sink<queue> m_queueSink;
  sink<std::vector<semaphore>> m_waitSemsSink;
};

}  // namespace sge::vulkan
