#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
namespace strobe::vk {

extern VkInstance g_instance;
extern VkDebugUtilsMessengerEXT g_debugger;
extern VkDevice g_device;
extern VkPhysicalDevice g_physicalDevice;
extern uint32_t g_gfxQueueFamilyIndex;
extern VkQueue g_gfxQueue;
extern uint32_t g_presentQueueFamilyIndex;
extern VkQueue g_presentQueue;
extern VkSurfaceKHR g_surface;
extern VkSwapchainKHR g_swapchain;
extern VkFormat g_swapchainFormat;
extern std::vector<VkImage> g_swapchainImages;
extern std::vector<VkImageView> g_swapchainImageViews;

void init();

void destroy();
}
