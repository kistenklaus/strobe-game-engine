#pragma once

#include "strb/apis/vulkan/CommandBuffer.hpp"
#include "strb/apis/vulkan/Semaphore.hpp"
#include "strb/apis/vulkan/Swapchain.hpp"
#include "strb/apis/vulkan/assert.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"
#include "strb/stl.hpp"

namespace strb::vulkan {

struct CommandBuffer;
struct Semaphore;
struct Swapchain;

struct Queue {
  friend struct Device;

 private:
  VkQueue queue;
  Queue(VkQueue queue) : queue(queue) {}

 public:
  inline operator VkQueue() { return this->queue; }
  void submit(const CommandBuffer& commandBuffer,
              const strb::vector<Semaphore>& waitSemaphores,
              const strb::vector<Semaphore>& signalSemaphores);
  void present(const Swapchain& swapchain, const uint32_t frameIndex,
               const strb::vector<Semaphore>& waitSemaphores);
};

}  // namespace strb::vulkan
