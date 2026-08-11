#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/gpu/vulkan/allocator.hpp"
#include "strobe/gpu/vulkan/context/select_queues.hpp"
#include <vulkan/vulkan.h>

namespace strobe::gpu::vulkan {

Vector<VkQueue, vulkan::allocator_ref>
get_queues(VkDevice device, span<const QueueLocation> locations, const vulkan::allocator_ref& alloc);

}
