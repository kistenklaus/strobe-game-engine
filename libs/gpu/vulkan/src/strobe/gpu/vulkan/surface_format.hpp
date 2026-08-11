#pragma once

#include <cstdint>
#include <type_traits>

namespace strobe::gpu::vulkan {

enum class SurfaceFormat : uint8_t {
  srgb8,
  unorm8,
};

} // namespace strobe::gpu::vulkan
