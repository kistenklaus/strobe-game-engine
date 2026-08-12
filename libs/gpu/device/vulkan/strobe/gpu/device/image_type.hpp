#pragma once

#include <cstdint>


namespace strobe::gpu {

enum class ImageType : uint8_t {
  image_1d,
  image_2d,
  image_3d,
};

}
