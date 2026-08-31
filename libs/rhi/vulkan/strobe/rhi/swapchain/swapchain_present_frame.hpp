#pragma once

#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include "strobe/rhi/swapchain/swapchain_generation.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"

namespace strobe::rhi {

struct SwapchainPresentFrame {
  explicit SwapchainPresentFrame(SwapchainGeneration generation,
                                 BinarySemaphore presentReady) noexcept
      : generation(std::move(generation)),
        presentReady(std::move(presentReady)) {}

  SwapchainGeneration generation;
  BinarySemaphore presentReady;
};

using swapchain_present_frame_pool =
    MPSCMonotonicPoolResource<sizeof(SwapchainPresentFrame),
                              alignof(SwapchainPresentFrame),
                              strobe::rhi::allocator_ref>;
using swapchain_present_frame_pool_ref =
    AllocatorReference<swapchain_present_frame_pool>;

} // namespace strobe::rhi
