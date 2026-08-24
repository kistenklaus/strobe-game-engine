#include "strobe/gpu/vulkan/debug_name.hpp"

#include "strobe/gpu/vulkan/context/debug_utils.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

void set_debug_name(Context *context, vulkan::BinarySemaphore sem,
                    const char *name) {

  const VkDebugUtilsObjectNameInfoEXT name_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_SEMAPHORE,
      .objectHandle = reinterpret_cast<uint64_t>(sem.handle),
      .pObjectName = name,
  };

  (void)vk_set_debug_utils_object_name(context->device(), &name_info);
}
void set_debug_name(Context *context, vulkan::TimelineSemaphore sem,
                    const char *name) {

  const VkDebugUtilsObjectNameInfoEXT name_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_SEMAPHORE,
      .objectHandle = reinterpret_cast<uint64_t>(sem.handle),
      .pObjectName = name,
  };

  (void)vk_set_debug_utils_object_name(context->device(), &name_info);
}
void set_debug_name(Context *context, vulkan::Buffer buf, const char *name) {

  const VkDebugUtilsObjectNameInfoEXT name_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_BUFFER,
      .objectHandle = reinterpret_cast<uint64_t>(buf.handle),
      .pObjectName = name,
  };

  (void)vk_set_debug_utils_object_name(context->device(), &name_info);
}
void set_debug_name(Context *context, vulkan::Image img, const char *name) {

  const VkDebugUtilsObjectNameInfoEXT name_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_IMAGE,
      .objectHandle = reinterpret_cast<uint64_t>(img.handle),
      .pObjectName = name,
  };

  (void)vk_set_debug_utils_object_name(context->device(), &name_info);
}
void set_debug_name(Context *context, vulkan::CommandBuffer cmd,
                    const char *name) {

  const VkDebugUtilsObjectNameInfoEXT name_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_COMMAND_BUFFER,
      .objectHandle = reinterpret_cast<uint64_t>(cmd.handle),
      .pObjectName = name,
  };

  (void)vk_set_debug_utils_object_name(context->device(), &name_info);
}
void set_debug_name(Context *context, vulkan::CommandPool pool,
                    const char *name) {

  const VkDebugUtilsObjectNameInfoEXT name_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
      .objectHandle = reinterpret_cast<uint64_t>(pool.handle),
      .pObjectName = name,
  };

  (void)vk_set_debug_utils_object_name(context->device(), &name_info);
}
void set_debug_name(Context *context, vulkan::Fence fence, const char *name) {

  const VkDebugUtilsObjectNameInfoEXT name_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_FENCE,
      .objectHandle = reinterpret_cast<uint64_t>(fence.handle),
      .pObjectName = name,
  };

  (void)vk_set_debug_utils_object_name(context->device(), &name_info);
}

void set_debug_name(Context *context, vulkan::ShaderObject shader, const char *name) {
  const VkDebugUtilsObjectNameInfoEXT name_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_SHADER_EXT,
      .objectHandle = reinterpret_cast<uint64_t>(shader.handle),
      .pObjectName = name,
  };

  (void)vk_set_debug_utils_object_name(context->device(), &name_info);

}

} // namespace strobe::gpu::vulkan
