#pragma once

#include "strobe/rhi/vulkan/binary_semaphore.hpp"

namespace strobe::rhi {

struct BinarySemaphoreNode {
  void *pool = nullptr;
  std::atomic<uint32_t> refCount{0};
  vulkan::BinarySemaphore semaphore{};
  BinarySemaphoreNode *next = nullptr;
};

} // namespace strobe::rhi
