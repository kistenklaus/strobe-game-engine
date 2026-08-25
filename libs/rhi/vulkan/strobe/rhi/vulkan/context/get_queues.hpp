#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/vulkan/context/select_queues.hpp"
#include <vulkan/vulkan.h>

namespace strobe::rhi::vulkan {

Vector<VkQueue, strobe::rhi::allocator_ref>
get_queues(VkDevice device, span<const QueueLocation> locations,
           const strobe::rhi::allocator_ref &alloc);

}
