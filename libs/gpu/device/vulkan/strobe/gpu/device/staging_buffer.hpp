#pragma once

#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/memory.hpp"

namespace strobe::gpu {

struct StagingBuffer {


  StagingBuffer(vulkan::Buffer buffer, vulkan::Memory memory,
                VkDeviceSize size, void* mapped) noexcept
      : buffer(buffer), memory(memory), size(size), mapped(mapped), next(nullptr) {}

  vulkan::Buffer buffer;
  vulkan::Memory memory;
  VkDeviceSize size;
  void* mapped;
  StagingBuffer* next = nullptr;
};


} // namespace strobe::gpu
