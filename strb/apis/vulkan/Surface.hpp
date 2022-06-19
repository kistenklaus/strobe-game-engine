#pragma once
#include "strb/apis/vulkan/Instance.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct Surface {
 private:
  const Instance *instance;
  VkSurfaceKHR surface;

 public:
  inline Surface() : instance(nullptr), surface(VK_NULL_HANDLE) {}
  Surface(const Instance &instance, const strb::window::Window &window);
  void destroy();
  inline operator VkSurfaceKHR() const { return this->surface; }
};

}  // namespace strb::vulkan
