#include "strobe/gpu/vulkan/context/get_queues.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

Vector<VkQueue, vulkan::allocator_ref>
get_queues(VkDevice device, span<const QueueLocation> locations,
           const allocator_ref& alloc) {

  Vector<VkQueue, vulkan::allocator_ref> queues{alloc};
  queues.resize(locations.size(), VK_NULL_HANDLE);

  for (uint32_t i = 0; i < locations.size(); ++i) {
    const auto &loc = locations[i];
    if (!loc) {
      continue;
    }
    assert(loc.family != QueueLocation::invalid);
    assert(loc.index != QueueLocation::invalid);
    vkGetDeviceQueue(device, loc.family, loc.index, &queues[i]);
  }
  return queues;
}

} // namespace strobe::gpu::vulkan
