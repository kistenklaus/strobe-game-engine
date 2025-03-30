#pragma once

#include <memory>
#include <span>
#include <vulkan/vulkan.hpp>

#include "renderer/vks/context/VksContextExtension.hpp"
#include "window/Window.hpp"
namespace strobe::renderer::vks {

class VksContextWindowSupportExtension final : public VksContextExtension {
 public:
  VksContextWindowSupportExtension(strobe::window::Window window) {
    auto windowExtensions = window.getRequiredVkInstanceExtensions();
    m_extensionNames.assign(windowExtensions.begin(), windowExtensions.end());
  }
  std::span<const char* const> requiredVkInstanceExtensions()
      const final override {
    return m_extensionNames;
  }

  std::span<const char* const> requiredVkInstanceLayers() const final override {
    return {};
  }

  std::span<const char* const> requiredVkDeviceExtensions() const final override {
    return m_deviceExtensions;
  }

  uint64_t acceptPhysicalDevice(
      const vk::SurfaceKHR surface,
      const VksPhysicalDeviceInfo& physicalDeviceInfo) const final override {
    if (physicalDeviceInfo.surfacePresentModes(surface).empty()) {
      return 0;
    }
    if (physicalDeviceInfo.surfaceFormats(surface).empty()) {
      return 0;
    }
    return 1;
  }

 private:
  std::vector<const char*> m_extensionNames;
  constexpr static std::array<const char*, 1> m_deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}  // namespace strobe::renderer::vks
