#include <vulkan/vulkan_core.h>

#include <stdexcept>

#include "vk.h"
#pragma one

namespace strobe::vk {

struct PipelineLayout {
 public:
  PipelineLayout() : m_handle(VK_NULL_HANDLE) {}
  static PipelineLayout create() {
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 0;
    createInfo.pSetLayouts = nullptr;
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges = nullptr;

    VkPipelineLayout handle;
    if (vkCreatePipelineLayout(g_device, &createInfo, nullptr, &handle) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create pipeline layout.");
    }
    return PipelineLayout(handle);
  }

  ~PipelineLayout() {
    if (m_handle != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(g_device, m_handle, nullptr);
    }
  }

  operator VkPipelineLayout() const { return m_handle; }

 private:
  explicit PipelineLayout(VkPipelineLayout h) : m_handle(h) {}
  VkPipelineLayout m_handle;
};

}  // namespace strobe::vk
