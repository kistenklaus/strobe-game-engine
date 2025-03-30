#pragma once

#include <memory>
#include <span>
#include <vulkan/vulkan.hpp>

#include "renderer/vks/context/VkPhysicalDeviceInfo.hpp"
#include "renderer/vks/context/VksContextExtensionLifetime.hpp"
namespace strobe::renderer::vks {

class VksContextExtension {
 public:
  virtual std::span<const char* const> requiredVkInstanceExtensions() const {
    return {};
  }

  virtual std::span<const char* const> requiredVkInstanceLayers() const {
    return {};
  }

  virtual void* appendVkInstanceCreateInfo(void* pnext) { return pnext; }

  virtual std::span<const char* const> requiredVkDeviceExtensions() const {
    return {};
  }

  virtual std::unique_ptr<VksContextExtensionLifetime> afterInstanceCreation(
      vk::Instance instance, const vk::detail::DispatchLoaderDynamic& dldi) {
    return nullptr;
  }

  virtual uint64_t acceptPhysicalDevice(
      vk::SurfaceKHR surface,
      const VksPhysicalDeviceInfo& physicalDeviceInfo) const {
    return 1;
  }

  virtual void appendQueueCreateInfo(
      std::vector<vk::DeviceQueueCreateInfo>& createInfos,
      std::vector<float>& queuePriorities, vk::SurfaceKHR surface,
      const VksPhysicalDeviceInfo& physicalDeviceInfo) {}

  virtual void enableFeature(const VksPhysicalDeviceInfo& physicalDeviceInfo,
                             vk::PhysicalDeviceFeatures& features) {}

  virtual const void* enableFeature2(
      const VksPhysicalDeviceInfo& physicalDeviceInfo, const void* pNext) {
    return pNext;
  }

  virtual void afterDeviceCreation(
      vk::Instance instance, const VksPhysicalDeviceInfo& physicalDeviceInfo,
      vk::Device device) {}

  bool enabled = false;
};

}  // namespace strobe::renderer::vks
