#pragma once

#include "strobe/gpu/vulkan/memory.hpp"
#include <atomic>
namespace strobe::gpu {

struct MemoryBinding {

  MemoryBinding(vulkan::Memory memory, VkDeviceSize offset) noexcept
      : memory(memory), offset(offset) {}

  explicit operator bool() const noexcept {
    return static_cast<bool>(
        std::atomic_ref{memory}.load(std::memory_order_acquire));
  }

  vulkan::Memory memory{};
  VkDeviceSize offset;
};

} // namespace strobe::gpu
