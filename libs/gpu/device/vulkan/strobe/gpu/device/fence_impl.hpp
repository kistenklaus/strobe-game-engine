#pragma once

#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/vulkan/fence.hpp"
namespace strobe::gpu {

struct FenceImpl {
  FenceImpl(Context context, vulkan::Fence fence) noexcept
      : context(std::move(context)), fence(fence) {}
  ~FenceImpl() noexcept {
    vulkan::destroy_fence(context.get(), fence);
  }

  Context context;
  vulkan::Fence fence;
};

} // namespace strobe::gpu
