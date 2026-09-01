#include "strobe/rhi/vulkan/surface.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include <stdexcept>

namespace strobe::rhi::vulkan {

Surface create_surface(Context *context, GLFWwindow *window) {
  assert(context->properties().surface);
  assert(window != nullptr);

  Surface surface;
  {
    ZoneScopedN("glfwCreateWindowSurface");
    VkResult result = glfwCreateWindowSurface(
        context->instance(), window, context->driver_alloc(), &surface.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to create glfw window surface");
    }
  }
  return surface;
}

void destroy_surface(Context *context, Surface surface) {
  assert(context->instance());
  assert(surface);
  {
    ZoneScopedN("vkDestroySurfaceKHR");
    vkDestroySurfaceKHR(context->instance(), surface.handle,
                        context->driver_alloc());
  }
}
SurfaceCapabilities query_surface_capabilities(Context *context,
                                               Surface surface) {

  assert(context != nullptr);
  assert(surface);

  VkSurfaceCapabilitiesKHR caps{};

  {
    ZoneScopedN("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        context->physicalDevice(), surface.handle, &caps);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to query Vulkan surface capabilities");
    }
  }

  return SurfaceCapabilities{
      .minImageCount = caps.minImageCount,
      .maxImageCount = caps.maxImageCount,
      .currentExtent = caps.currentExtent,
      .minImageExtent = caps.minImageExtent,
      .maxImageExtent = caps.maxImageExtent,
      .maxImageArrayLayers = caps.maxImageArrayLayers,
      .supportedTransforms = caps.supportedTransforms,
      .currentTransform = caps.currentTransform,
      .supportedCompositeAlpha = caps.supportedCompositeAlpha,
      .supportedUsageFlags = caps.supportedUsageFlags,
  };
}

} // namespace strobe::rhi::vulkan
