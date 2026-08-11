#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/core/containers/span.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"
#include "strobe/gpu/vulkan/context/debug_utils.hpp"
#include "strobe/gpu/vulkan/context/get_queues.hpp"
#include "strobe/gpu/vulkan/context/instance.hpp"
#include "strobe/gpu/vulkan/context/logical_device.hpp"
#include "strobe/gpu/vulkan/context/select_physical_device.hpp"
#include "strobe/gpu/vulkan/context/select_queues.hpp"
#include "strobe/gpu/vulkan/context/vma.hpp"
#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

Context::Context(const ContextCreateInfo &info, const allocator &alloc) noexcept
    : m_alloc{alloc}, m_driverAlloc(), m_props{},
      m_instance(create_instance(&info, &m_props, &m_driverAlloc)),
      m_messenger(
          create_debug_utils_messenger(m_instance, &m_props, &m_driverAlloc)),
      m_physicalDevice(select_physical_device(m_instance, &info, &m_props)),
      m_deviceInfo(DeviceInfo<allocator_ref>::query(
          m_instance, m_physicalDevice, m_props.api_version, &m_alloc)),
      m_queueLocations(select_queues(&m_deviceInfo, &info, &m_alloc)),
      m_device(create_logical_device(m_physicalDevice, &m_deviceInfo,
                                     m_queueLocations, &m_props, &info,
                                     &m_driverAlloc)),
      m_queues(get_queues(m_device, m_queueLocations, &m_alloc)),
      m_vma(create_vma(m_instance, m_physicalDevice, m_device,
                       m_props.api_version, &m_driverAlloc)) {}

Context::~Context() noexcept {

  vkDeviceWaitIdle(m_device);

  vmaDestroyAllocator(m_vma);

  // logical device.
  assert(m_device != nullptr);
  vkDestroyDevice(m_device, m_driverAlloc.callbacks());
  m_device = VK_NULL_HANDLE;

  // physical device
  m_physicalDevice = VK_NULL_HANDLE;

  // instance
  assert(m_instance != VK_NULL_HANDLE);
  if (m_messenger != VK_NULL_HANDLE) {
    vk_destroy_debug_utils_messenger(m_instance, m_messenger,
                                     m_driverAlloc.callbacks());
  }
  vkDestroyInstance(m_instance, m_driverAlloc.callbacks());

  m_messenger = VK_NULL_HANDLE;
  m_instance = VK_NULL_HANDLE;
}

// SAN suppressions
#if !defined(NDEBUGG)
extern "C" const char *__lsan_default_options() {
  return "print_suppressions=0";
}
extern "C" const char *__lsan_default_suppressions() {
  return R"(
leak:libnvidia-glcore.so
)";
}
#endif

} // namespace strobe::gpu::vulkan
