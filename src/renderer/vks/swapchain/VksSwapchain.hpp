#pragma once

#include <vulkan/vulkan.hpp>

#include "renderer/vks/context/VksContext.hpp"
#include "renderer/vks/swapchain/VksSwapchainControlBlock.hpp"

namespace strobe::renderer::vks {

class VksSwapchain {
 public:
  static std::expected<VksSwapchain, vk::Result> tryCreate(
      strobe::window::Window& window, const VksContext& context);
  void destroy(const VksContext& context);
  VksSwapchain() : m_controlBlock(nullptr) {}

 private:
  VksSwapchain(VksSwapchainControlBlock* controlBlock)
      : m_controlBlock(controlBlock) {}

 private:
  VksSwapchainControlBlock* m_controlBlock;
};

}  // namespace strobe::renderer::vks
