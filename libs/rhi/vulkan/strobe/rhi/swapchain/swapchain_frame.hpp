#pragma once

#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"

namespace strobe::rhi {

struct SwapchainFrame {
  Image image{};
  ImageView view{};
  BinarySemaphore imageAvailable{}; // passed to acquire
};

} // namespace strobe::rhi
