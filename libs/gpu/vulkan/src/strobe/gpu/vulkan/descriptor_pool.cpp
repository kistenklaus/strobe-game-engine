#include "strobe/gpu/vulkan/descriptor_pool.hpp"

#include <stdexcept>

namespace strobe::gpu::vulkan {

DescriptorPool create_descriptor_pool(Context *context,
                                      const DescriptorPoolInfo &info) {
  assert(context != nullptr);
  VkDescriptorPoolCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .maxSets = info.maxSets,
      .poolSizeCount = static_cast<uint32_t>(info.poolSizes.size()),
      .pPoolSizes = info.poolSizes.data(),
  };

  DescriptorPool pool{};
  VkResult result = vkCreateDescriptorPool(
      context->device(), &createInfo, context->driver_alloc(), &pool.handle);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool");
  }
  return pool;
}
void destroy_descriptor_pool(Context *context, DescriptorPool pool) noexcept {
  assert(context != nullptr);
  assert(pool);
  vkDestroyDescriptorPool(context->device(), pool.handle,
                          context->driver_alloc());
}
void reset_descriptor_pool(Context *context, DescriptorPool pool) {
  assert(context != nullptr);
  assert(pool);
  VkResult result = vkResetDescriptorPool(context->device(), pool.handle, 0);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to reset descriptor pool");
  }
}

} // namespace strobe::gpu::vulkan
