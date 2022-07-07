#pragma once
#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VRendererBackend.hpp"

namespace sge::vulkan {

enum VulkanQueueAcquireTarget { GRAPHICS_QUEUE, COMPUTE_QUEUE, TRANSFER_QUEUE };

class VAcquireQueuePass : public RenderPass {
 public:
  VAcquireQueuePass(VRendererBackend* renderer, const std::string name,
                    VulkanQueueAcquireTarget queueType = GRAPHICS_QUEUE);

  void execute() override;

  u32 getQueueSource() const { return m_queueSource; }

 private:
  const u32 m_queueType;
  const u32 m_queueSource;
  queue m_queueHandle;
};

}  // namespace sge::vulkan
