#pragma once

#include "strobe/gpu/vulkan/buffer.hpp"

namespace strobe::gpu {

struct BufferBinding {
  vulkan::Buffer buffer{};
  VkDeviceSize offset = 0;
  VkDeviceSize size = 0;
  void* mapped;

  explicit operator bool() const noexcept {
    return static_cast<bool>(buffer);
  }
};

}
