#include "./VksContextDebugUtilsExtension.hpp"

#include <memory>
#include <stdexcept>

#include "renderer/vks/context/VksContextExtensionLifetime.hpp"
#include "spdlog/spdlog.h"

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

namespace strobe::renderer::vks {

class VksContextDebugUtilsExtensionLifetime
    : public VksContextExtensionLifetime {
 public:
  vk::DebugUtilsMessengerEXT messenger = nullptr;
  void destroy(vk::Instance instance, const vk::detail::DispatchLoaderDynamic& dldi) final override {
    instance.destroyDebugUtilsMessengerEXT(messenger, nullptr, dldi);
  }
};

VksContextDebugUtilsExtension::VksContextDebugUtilsExtension(
    MessageCallback callback, void* userData,
    vk::DebugUtilsMessageSeverityFlagsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type) {
  m_createInfo.messageSeverity = severity;
  m_createInfo.messageType = type;
  m_createInfo.pfnUserCallback = callback;
  m_createInfo.pUserData = userData;
}

std::span<const char* const>
VksContextDebugUtilsExtension::requiredVkInstanceExtensions() const {
  return m_extensions;
}

std::span<const char* const>
VksContextDebugUtilsExtension::requiredVkInstanceLayers() const {
  return m_layers;
}
void* VksContextDebugUtilsExtension::appendVkInstanceCreateInfo(void* pnext) {
  m_createInfo.pNext = pnext;
  return &m_createInfo;
}
std::unique_ptr<VksContextExtensionLifetime>
VksContextDebugUtilsExtension::afterInstanceCreation(
    vk::Instance instance, const vk::detail::DispatchLoaderDynamic& dldi) {
  auto lifetime = std::make_unique<VksContextDebugUtilsExtensionLifetime>();
  vk::Result result = instance.createDebugUtilsMessengerEXT(
      &m_createInfo, nullptr, &lifetime->messenger, dldi);
  if (result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to create debug utils messenger");
  }
  return lifetime;
}

}  // namespace strobe::renderer::vks
