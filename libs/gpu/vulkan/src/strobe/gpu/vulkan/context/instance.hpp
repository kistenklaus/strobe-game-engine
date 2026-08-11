#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/gpu/vulkan/context/context_properties.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"
#include "strobe/gpu/vulkan/context/debug_utils.hpp"
#include "strobe/gpu/vulkan/context/driver_alloc.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fmt/format.h>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace strobe::gpu::vulkan {

VkInstance create_instance(const ContextCreateInfo *info,
                           ContextProperties *props, DriverAlloc *alloc);

} // namespace strobe::gpu::vulkan
