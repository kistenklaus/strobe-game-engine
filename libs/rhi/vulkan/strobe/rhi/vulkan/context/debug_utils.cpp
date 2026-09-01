#include "strobe/rhi/vulkan/context/debug_utils.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity,
               [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
               const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
               [[maybe_unused]] void *user_data) {

  fmt::println("[Vulkan] {}", callback_data->pMessage);

  return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info() noexcept {
  return VkDebugUtilsMessengerCreateInfoEXT{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                         // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = &debug_callback,
      .pUserData = nullptr,
  };
}

VkResult vk_create_debug_utils_messenger(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info,
    const VkAllocationCallbacks *allocator,
    VkDebugUtilsMessengerEXT *messenger) noexcept {
  const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  if (function == nullptr) {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
  return function(instance, create_info, allocator, messenger);
}

void vk_destroy_debug_utils_messenger(
    VkInstance instance, VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks *allocator) noexcept {
  if (messenger == VK_NULL_HANDLE) {
    return;
  }
  const auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (function != nullptr) {
    function(instance, messenger, allocator);
  }
}

VkDebugUtilsMessengerEXT
create_debug_utils_messenger(VkInstance instance, ContextProperties *props,
                             DriverAlloc *alloc) {
  ZoneScopedN("context/create-debug-utils-messenger");
  VkDebugUtilsMessengerCreateInfoEXT createInfo = debug_messenger_create_info();
  if (props->debug_utils) {
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    ZoneScopedN("vkCreateDebugUtilsMessenger");
    VkResult result = vk_create_debug_utils_messenger(
        instance, &createInfo, alloc->callbacks(), &messenger);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to create debug utils messenger");
    }
    return messenger;
  } else {
    return VK_NULL_HANDLE;
  }
}

VkResult vk_set_debug_utils_object_name(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT *name_info) noexcept {
  const auto function =
      reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
          vkGetDeviceProcAddr(
              device,
              "vkSetDebugUtilsObjectNameEXT"));

  if (function == nullptr) {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  return function(device, name_info);
}

} 
