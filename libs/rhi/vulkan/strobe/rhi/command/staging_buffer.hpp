#pragma once

#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/memory.hpp"

namespace strobe::rhi {

struct StagingBuffer {

  explicit StagingBuffer(vulkan::Buffer buffer, vulkan::Memory memory,
                VkDeviceSize size, void* mapped) noexcept
      : buffer(buffer), memory(memory), size(size), mapped(mapped), next(nullptr) {}

  vulkan::Buffer buffer;
  vulkan::Memory memory;
  VkDeviceSize size;
  void* mapped;
  StagingBuffer* next = nullptr;
};


} // namespace strobe::rhi
