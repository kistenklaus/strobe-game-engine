#pragma once
#include "strb/apis/vulkan/vulkan.lib.hpp"
#include "strb/stl.hpp"
#include "strb/window.hpp"

namespace strb::vulkan {

struct Instance {
 private:
  VkInstance instance;

 public:
  inline Instance() : instance(VK_NULL_HANDLE) {}
  Instance(const strb::string &applicationName, const strb::string &engineName,
           const strb::tuple3i &applicationVersion,
           const strb::tuple3i &engineVersion,
           const strb::tuple3i &vulkanApiVersion,
           const strb::window::WindowApi windowApi,
           const strb::vector<strb::string> extentions,
           const strb::vector<strb::string> layers);
  inline operator VkInstance() const { return this->instance; }
  void destroy();
};

}  // namespace strb::vulkan
