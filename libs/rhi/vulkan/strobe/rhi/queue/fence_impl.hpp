#pragma once

#include "strobe/rhi/device/context.hpp"
#include "strobe/rhi/vulkan/fence.hpp"

namespace strobe::rhi {

struct FenceImpl {
  FenceImpl(Context context, vulkan::Fence fence) noexcept
      : context(std::move(context)), fence(fence) {}
  ~FenceImpl() noexcept {
    vulkan::destroy_fence(context.ctx(), fence);
  }

  Context context;
  vulkan::Fence fence;
};

} // namespace strobe::rhi
