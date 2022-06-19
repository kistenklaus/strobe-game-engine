#pragma once
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"
#include "strb/stl.hpp"

namespace strb::vulkan {

struct Shader {
 private:
  const Device* device;
  VkShaderModule shader;
  strb::vector<char> sourceCode;

 public:
  inline Shader() : device(nullptr), shader(VK_NULL_HANDLE) {}
  Shader(const Device& device, const strb::vector<char> sourceCode);
  void destroy();
  inline operator VkShaderModule() const { return this->shader; }
};

}  // namespace strb::vulkan
