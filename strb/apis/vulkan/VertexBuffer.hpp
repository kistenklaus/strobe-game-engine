#pragma once
#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/assert.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"
#include "strb/stl.hpp"

namespace strb::vulkan {

struct VertexBuffer {
 private:
  const Device* device;
  VkBuffer vertexBuffer;
  VkMemoryRequirements memory;

 public:
  inline VertexBuffer() : device(nullptr), vertexBuffer(VK_NULL_HANDLE) {}
  VertexBuffer(const Device& device);
  void destroy();
};

}  // namespace strb::vulkan
