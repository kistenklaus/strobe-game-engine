#pragma once

#include <cstdint>
namespace strobe::rhi {

enum class ImageAspect : uint8_t {
  color,
  depth,
  stencil,
  depth_stencil,
};

}
