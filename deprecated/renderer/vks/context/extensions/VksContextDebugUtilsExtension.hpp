#pragma once

#include <vulkan/vulkan_core.h>

#include <functional>
#include <span>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

#include "renderer/vks/context/VksContextExtension.hpp"
namespace strobe::renderer::vks {

class VksContextDebugUtilsExtension final : public VksContextExtension {
 public:
  using MessageCallback = vk::PFN_DebugUtilsMessengerCallbackEXT;

  explicit VksContextDebugUtilsExtension(
      MessageCallback callback, void* userData = nullptr,
      vk::DebugUtilsMessageSeverityFlagsEXT severity =
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
      vk::DebugUtilsMessageTypeFlagsEXT type =
          vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
          vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

  std::span<const char* const> requiredVkInstanceExtensions()
      const final override;

  std::span<const char* const> requiredVkInstanceLayers() const final override;

  void* appendVkInstanceCreateInfo(void* pnext) final override;

  std::unique_ptr<VksContextExtensionLifetime> afterInstanceCreation(
      vk::Instance instance,
      const vk::detail::DispatchLoaderDynamic& dldi) final override;

 private:
  constexpr static std::array<const char*, 1> m_layers = {
      "VK_LAYER_KHRONOS_validation"};

  constexpr static std::array<const char*, 1> m_extensions = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

  vk::DebugUtilsMessengerCreateInfoEXT m_createInfo;
};

}  // namespace strobe::renderer::vks
