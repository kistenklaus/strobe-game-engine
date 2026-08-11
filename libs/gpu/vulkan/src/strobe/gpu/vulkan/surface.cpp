#include "strobe/gpu/vulkan/surface.hpp"
#include <stdexcept>

namespace strobe::gpu::vulkan {

Surface create_surface(Context *context, GLFWwindow *window) {
  if (!context->properties().surface) {
    throw std::runtime_error("surface is not supported by this context");
  }
  assert(window != nullptr);

  Surface surface;
  VkResult result = glfwCreateWindowSurface(context->instance(), window,
                                            context->driver_alloc(), &surface.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create glfw window surface");
  }
  return surface;
}

void destroy_surface(Context *context, Surface surface) {
  assert(context->instance());
  assert(surface);
  vkDestroySurfaceKHR(context->instance(), surface.handle,
                      context->driver_alloc());
}

} // namespace strobe::gpu::vulkan
