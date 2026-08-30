#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/vulkan/surface.hpp"

namespace strobe::rhi {

struct SurfaceImpl {

  SurfaceImpl(Context context, vulkan::Surface surface) noexcept
      : context(std::move(context)), surface(surface) {}

  ~SurfaceImpl() noexcept { vulkan::destroy_surface(context.ctx(), surface); }

  Context context;
  vulkan::Surface surface;
};

} // namespace strobe::rhi
