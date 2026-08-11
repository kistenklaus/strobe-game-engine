#pragma once

#include "strobe/core/containers/string.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"

#include <vulkan/vulkan.h>

namespace strobe::gpu::vulkan {

template <Allocator Alloc = strobe::Mallocator> struct DeviceExtension {
  String<Alloc> name;
  uint32_t specVersion;
};

template <Allocator Alloc = strobe::Mallocator>
static Vector<DeviceExtension<Alloc>, Alloc>
query_device_extensions(VkPhysicalDevice physicalDevice,
                        const Alloc &alloc = {}) {
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, 1 << 10>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};

  Vector<VkExtensionProperties, scratch_allocator_ref> native{&scratch};
  while (true) {
    std::uint32_t count = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &count, nullptr);
    if (result != VK_SUCCESS) {
      throw std::runtime_error{"Failed to enumerate device extensions"};
    }
    native.resize(count);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                                  &count, native.data());
    native.resize(count);
    if (result == VK_SUCCESS) {
      break;
    }
    if (result != VK_INCOMPLETE) {
      throw std::runtime_error{"Failed to enumerate device extensions"};
    }
  }
  Vector<DeviceExtension<Alloc>, Alloc> extensions{alloc};
  extensions.reserve(native.size());
  for (const auto &extension : native) {
    extensions.push_back(DeviceExtension<Alloc>{
        .name = {extension.extensionName, alloc},
        .specVersion = extension.specVersion,
    });
  }
  return extensions;
}

} // namespace strobe::gpu::vulkan
