#pragma once

#include <cstdint>
namespace strobe::gpu {

enum class ImageAspect : uint8_t {
  color,
  depth,
  stencil,
  depth_stencil,
};

}
