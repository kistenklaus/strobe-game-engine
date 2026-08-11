#include "strobe/gpu/vulkan/fence.hpp"
#include <stdexcept>

namespace strobe::gpu::vulkan {

Fence create_fence(Context *context, const FenceInfo &info) {
  assert(context != nullptr);

  VkFenceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
  };
  Fence fence;
  VkResult result = vkCreateFence(context->device(), &createInfo,
                                  context->driver_alloc(), &fence.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create fence");
  }
  return fence;
}

void destroy_fence(Context *context, Fence fence) noexcept {
  assert(context != nullptr);
  assert(fence);
  vkDestroyFence(context->device(), fence.handle, context->driver_alloc());
}

bool wait_for_fence(Context *context, Fence fence, uint64_t timeout) {
  assert(timeout != 0);
  VkResult result =
      vkWaitForFences(context->device(), 1, &fence.handle, true, timeout);

  if (result == VK_TIMEOUT) {
    return false;
  }
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to wait for fence");
  }
  return true;
}

void reset_fence(Context *context, Fence fence) {
  VkResult result = vkResetFences(context->device(), 1, &fence.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to reset fence");
  }
}

bool is_fence_signaled(Context *context, Fence fence) {
  assert(context != nullptr);
  assert(fence);

  VkResult result = vkGetFenceStatus(context->device(), fence.handle);

  if (result == VK_SUCCESS) {
    return true;
  } else if (result == VK_NOT_READY) {
    return false;
  } else {
    throw std::runtime_error("Failed to query vulkan fence status");
  }
}

} // namespace strobe::gpu::vulkan
