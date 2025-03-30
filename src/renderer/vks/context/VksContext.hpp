#pragma once

#include <cstring>
#include <expected>
#include <print>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_funcs.hpp>

#include "renderer/vks/context/VksContextControlBlock.hpp"
#include "renderer/vks/context/VksContextExtension.hpp"
#include "window/Window.hpp"

namespace strobe::renderer::vks {

class VksContext {
 public:
  static std::expected<VksContext, vk::Result> tryCreate(
      strobe::window::Window window, uint32_t apiVersion,
      std::span<VksContextExtension*> extensions = {},
      std::span<VksContextExtension*> optionalExtensions = {});

 public:
  void destroy();

  const VksPhysicalDeviceInfo& physicalDeviceInfo() const {
    return m_controlBlock->physicalDeviceInfo;
  }

  const VksQueueInfo& getGraphicsQueue() const {
    return m_controlBlock->graphicsQueues.at(0);
  }

  const VksQueueInfo& getPresentQueue() const {
    return m_controlBlock->presentQueues.at(0);
  }

  const VksQueueInfo& getTransferQueue() const {
    return m_controlBlock->transferQueues.at(0);
  }

  const vk::SurfaceKHR surface() const {
    return m_controlBlock->surface;
  }

  VksContext() = default;

 private:
  VksContext(vk::Instance instance, vk::Device device,
             VksContextControlBlock* controlBlock);

 public:
  vk::Instance instance;
  vk::Device device;

 private:
  VksContextControlBlock* m_controlBlock;
};

}  // namespace strobe::renderer::vks
