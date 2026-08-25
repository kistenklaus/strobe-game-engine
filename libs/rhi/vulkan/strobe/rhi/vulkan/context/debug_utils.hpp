#pragma once

#include "strobe/rhi/vulkan/context/context_properties.hpp"
#include "strobe/rhi/vulkan/context/driver_alloc.hpp"
#include <fmt/printf.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity,
               [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
               const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
               [[maybe_unused]] void *user_data);

VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info() noexcept;

VkResult vk_create_debug_utils_messenger(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info,
    const VkAllocationCallbacks *allocator,
    VkDebugUtilsMessengerEXT *messenger) noexcept;

void vk_destroy_debug_utils_messenger(
    VkInstance instance, VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks *allocator) noexcept;

VkDebugUtilsMessengerEXT create_debug_utils_messenger(VkInstance instance,
                                                      ContextProperties *props,
                                                      DriverAlloc *alloc);

VkResult vk_set_debug_utils_object_name(
    VkDevice device, const VkDebugUtilsObjectNameInfoEXT *name_info) noexcept;

} // namespace strobe::rhi::vulkan
