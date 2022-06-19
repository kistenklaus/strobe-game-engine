#include "strb/apis/vulkan/Surface.hpp"

#include <cassert>

#include "strb/apis/glfw.hpp"
#include "strb/apis/vulkan/assert.hpp"

namespace strb::vulkan {

Surface::Surface(const Instance &instance, const strb::window::Window &window)
    : instance(&instance) {
  assert(instance != VK_NULL_HANDLE);
  if (window.windowApi == strb::window::WINDOW_API_GLFW) {
    const strb::glfw::Window *glfwWindow =
        static_cast<const strb::glfw::Window *>(&window);
    VkResult result = glfwCreateWindowSurface(
        instance, const_cast<GLFWwindow *>(glfwWindow->pointer()), nullptr,
        &this->surface);
    ASSERT_VKRESULT(result);
  } else {
    throw std::runtime_error(
        "The window library doesn't support creation of VkSurfaceKHR");
  }
}
void Surface::destroy() {
  assert(this->surface != VK_NULL_HANDLE);
  assert(this->instance != nullptr);
  vkDestroySurfaceKHR(*this->instance, this->surface, nullptr);
  dexec(this->surface = VK_NULL_HANDLE);
  dexec(this->instance = nullptr);
}

}  // namespace strb::vulkan
