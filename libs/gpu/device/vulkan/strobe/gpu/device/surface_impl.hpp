#pragma once

#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/vulkan/surface.hpp"
namespace strobe::gpu {

struct SurfaceImpl {

  SurfaceImpl(Context context, vulkan::Surface surface) noexcept
      : context(std::move(context)), surface(surface) {}

  ~SurfaceImpl() noexcept { vulkan::destroy_surface(context.get(), surface); }

  Context context;
  vulkan::Surface surface;
};

} // namespace strobe::gpu
