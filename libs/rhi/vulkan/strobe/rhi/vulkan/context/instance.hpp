#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/vulkan/context/context_properties.hpp"
#include "strobe/rhi/vulkan/context/create_info.hpp"
#include "strobe/rhi/vulkan/context/debug_utils.hpp"
#include "strobe/rhi/vulkan/context/driver_alloc.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fmt/format.h>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace strobe::rhi::vulkan {

VkInstance create_instance(const ContextCreateInfo *info,
                           ContextProperties *props, DriverAlloc *alloc);

} // namespace strobe::rhi::vulkan
