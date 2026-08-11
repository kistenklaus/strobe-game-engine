#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct QueueFamilyProperties {
  // VK_API_VERSION_1_0
  VkQueueFlags queueFlags;
  uint32_t queueCount;
  uint32_t timestampValidBits;
  VkExtent3D minImageTransferGranularity;

  // VK_API_VERSION_1_1
  // ...

  // VK_API_VERSION_1_2
  // ...

  // VK_API_VERSION_1_3
  // ...

  // VK_API_VERSION_1_4
  // ...

  // GLFW presentation support
  bool presentationSupport;
};

template <Allocator Alloc>
static Vector<QueueFamilyProperties, Alloc>
query_queue_family_properties(VkInstance instance,
                              VkPhysicalDevice physicalDevice,
                              const Alloc &alloc = {}) noexcept {
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, 1 << 14>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};

  Vector<VkQueueFamilyProperties2, scratch_allocator_ref> native{&scratch};

  {
    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount,
                                              nullptr);

    native.resize(queueFamilyCount, {});

    for (auto &props : native) {
      props.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
      props.pNext = nullptr;

      // Later extend here.
      // Any extension structs can be allocated from scratch.
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount,
                                              native.data());

    native.resize(queueFamilyCount);
  }

  Vector<QueueFamilyProperties, Alloc> queueFamilyProperties{alloc};
  queueFamilyProperties.reserve(native.size());

  for (uint32_t qfi = 0; qfi < native.size(); ++qfi) {
    const auto &props = native[qfi];

    const bool presentationSupport =
        glfwGetPhysicalDevicePresentationSupport(instance, physicalDevice,
                                                 qfi) == GLFW_TRUE;

    queueFamilyProperties.push_back(QueueFamilyProperties{
        .queueFlags = props.queueFamilyProperties.queueFlags,

        .queueCount = props.queueFamilyProperties.queueCount,

        .timestampValidBits = props.queueFamilyProperties.timestampValidBits,

        .minImageTransferGranularity =
            props.queueFamilyProperties.minImageTransferGranularity,

        .presentationSupport = presentationSupport,
    });
  }

  return queueFamilyProperties;
}

} // namespace strobe::gpu::vulkan
