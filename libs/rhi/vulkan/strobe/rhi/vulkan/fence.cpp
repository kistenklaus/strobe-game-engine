#include "strobe/rhi/vulkan/fence.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include <stdexcept>

namespace strobe::rhi::vulkan {

Fence create_fence(Context *context, const FenceInfo &info) {
  assert(context != nullptr);

  VkFenceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
  };
  Fence fence;
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkCreateFence");
#endif
    VkResult result = vkCreateFence(context->device(), &createInfo,
                                    context->driver_alloc(), &fence.handle);
    if (result != VK_SUCCESS) {
      vulkan_error(result, "Failed to create fence");
    }
  }
  return fence;
}

void destroy_fence(Context *context, Fence fence) noexcept {
  assert(context != nullptr);
  assert(fence);
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkDestroyFence");
#endif
    vkDestroyFence(context->device(), fence.handle, context->driver_alloc());
  }
}

bool wait_for_fence(Context *context, Fence fence, uint64_t timeout) {
  assert(timeout != 0);

#ifdef STROBE_RHI_TRACE_VK
  ZoneScopedN("vkWaitForFences");
#endif
  VkResult result =
      vkWaitForFences(context->device(), 1, &fence.handle, true, timeout);
  if (result == VK_TIMEOUT) {
    return false;
  }
  if (result != VK_SUCCESS) {
    vulkan_error(result,"Failed to wait for fence");
  }
  return true;
}

void reset_fence(Context *context, Fence fence) {
#ifdef STROBE_RHI_TRACE_VK
  ZoneScopedN("vkResetFences");
#endif
  VkResult result = vkResetFences(context->device(), 1, &fence.handle);
  if (result != VK_SUCCESS) {
    vulkan_error(result, "Failed to reset fence");
  }
}

bool is_fence_signaled(Context *context, Fence fence) {
  assert(context != nullptr);
  assert(fence);

#ifdef STROBE_RHI_TRACE_VK
  ZoneScopedN("vkGetFenceStatus");
#endif
  VkResult result = vkGetFenceStatus(context->device(), fence.handle);

  if (result == VK_SUCCESS) {
    return true;
  } else if (result == VK_NOT_READY) {
    return false;
  } else {
    vulkan_error(result, "Failed to query vulkan fence status");
  }
}

} // namespace strobe::rhi::vulkan
