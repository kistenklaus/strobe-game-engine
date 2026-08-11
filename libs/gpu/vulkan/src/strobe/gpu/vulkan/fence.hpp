#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include <limits>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct Fence {
  VkFence handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct FenceInfo {
  VkFenceCreateFlags flags = 0;
};

Fence create_fence(Context *context, const FenceInfo& info = {});

void destroy_fence(Context *context, Fence fence) noexcept;

bool wait_for_fence(Context *context, Fence fence,
                    uint64_t timeout = std::numeric_limits<uint64_t>::max());

void reset_fence(Context *context, Fence fence);

bool is_fence_signaled(Context *context, Fence fence);

} // namespace strobe::gpu::vulkan
