#include "strobe/rhi/vulkan/context/get_queues.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

Vector<VkQueue, strobe::rhi::allocator_ref>
get_queues(VkDevice device, span<const QueueLocation> locations,
           const allocator_ref &alloc) {
  ZoneScopedN("context/query-queues");
  Vector<VkQueue, strobe::rhi::allocator_ref> queues{alloc};
  queues.resize(locations.size(), VK_NULL_HANDLE);
  for (uint32_t i = 0; i < locations.size(); ++i) {
    const auto &loc = locations[i];
    if (!loc) {
      continue;
    }
    assert(loc.family != QueueLocation::invalid);
    assert(loc.index != QueueLocation::invalid);
    ZoneScopedN("vkGetDeviceQueue");
    vkGetDeviceQueue(device, loc.family, loc.index, &queues[i]);
  }
  return queues;
}

} // namespace strobe::rhi::vulkan
