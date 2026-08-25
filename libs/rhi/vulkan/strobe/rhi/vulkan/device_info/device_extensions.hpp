#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/core/containers/string.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/allocator.hpp"

#include <vulkan/vulkan.h>

namespace strobe::rhi::vulkan {

struct DeviceExtension {
  String<strobe::rhi::allocator_ref> name;
  uint32_t specVersion;
};

namespace details {

Vector<DeviceExtension, strobe::rhi::allocator_ref>
query_device_extensions(VkPhysicalDevice physicalDevice,
                        const strobe::rhi::allocator_ref &alloc);

bool supports_extension(span<const DeviceExtension> extensions,
                        const char *required);

} // namespace details

} // namespace strobe::rhi::vulkan
