#pragma once
#include <string>

#include "renderer/RenderPass.hpp"
#include "renderer/vulkan/VulkanRendererBackend.hpp"

namespace sge::vulkan {

enum VulkanQueueAcquireTarget { GRAPHICS_QUEUE, COMPUTE_QUEUE, TRANSFER_QUEUE };

class VulkanAcquireQueuePass : public RenderPass {
 public:
  VulkanAcquireQueuePass(VulkanRendererBackend* renderer,
                         const std::string name,
                         VulkanQueueAcquireTarget queueType = GRAPHICS_QUEUE);

  void execute() override;

  u32 getQueueSource() const { return m_queueSource; }

 private:
  const u32 m_queueType;
  const u32 m_queueSource;
  u32 m_queueIndex;
};

}  // namespace sge::vulkan
