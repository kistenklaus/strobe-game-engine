#pragma once

#include <cstdint>
namespace strobe::rhi {

enum class ImageViewType : uint8_t {
  none,
  image_1d,
  image_2d,
  image_3d,
  image_1d_array,
  image_2d_array,
  cube,
  cube_array,
};

}
