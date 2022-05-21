#pragma once
#include <cassert>

#include "strb/apis/vulkan/Device.hpp"
#include "strb/apis/vulkan/vulkan.lib.hpp"

namespace strb::vulkan {

struct Device;

struct Semaphore {
 private:
  const Device* device;
  VkSemaphore semaphore;

 public:
  inline Semaphore() : device(nullptr), semaphore(VK_NULL_HANDLE) {}
  Semaphore(const Device& device);
  inline operator VkSemaphore() const {
    assert(this->semaphore != VK_NULL_HANDLE);
    assert(this->device != nullptr);
    return this->semaphore;
  }
  void destroy();
};

}  // namespace strb::vulkan
