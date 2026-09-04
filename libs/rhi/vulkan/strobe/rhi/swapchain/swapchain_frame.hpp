#pragma once

#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/sync/fence.hpp"

namespace strobe::rhi {

struct SwapchainFrame {
  Image image{};
  ImageView view{};
  BinarySemaphore imageAvailable{}; // passed to acquire
  Fence presentFence{};
};

} // namespace strobe::rhi
