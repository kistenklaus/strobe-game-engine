#include "strobe/rhi/vulkan/context/context.hpp"
#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/vulkan/context/create_info.hpp"
#include "strobe/rhi/vulkan/context/debug_utils.hpp"
#include "strobe/rhi/vulkan/context/get_queues.hpp"
#include "strobe/rhi/vulkan/context/instance.hpp"
#include "strobe/rhi/vulkan/context/logical_device.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include "strobe/rhi/vulkan/context/select_physical_device.hpp"
#include "strobe/rhi/vulkan/context/select_queues.hpp"
#include "strobe/rhi/vulkan/context/vma.hpp"
#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

Context::Context(const ContextCreateInfo &info,
                 const strobe::rhi::allocator_ref &alloc) noexcept
    : m_alloc{alloc}, m_driverAlloc(), m_props{},
      m_instance(create_instance(&info, &m_props, &m_driverAlloc)),
      m_messenger(
          create_debug_utils_messenger(m_instance, &m_props, &m_driverAlloc)),
      m_physicalDevice(
          select_physical_device(m_instance, &info, &m_props, m_alloc)),
      m_deviceInfo(DeviceInfo::query(m_instance, m_physicalDevice,
                                     m_props.api_version, m_alloc)),
      m_queueLocations(select_queues(&m_deviceInfo, &info, m_alloc)),
      m_device(create_logical_device(m_physicalDevice, &m_deviceInfo,
                                     m_queueLocations, &m_props, &info,
                                     &m_driverAlloc)),
      m_queues(get_queues(m_device, m_queueLocations, m_alloc)),
      m_vma(create_vma(m_instance, m_physicalDevice, m_device,
                       m_props.api_version, &m_driverAlloc)),
      m_pnf(load_pnf_functions(m_instance, m_device, m_props)) {}

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

} // namespace strobe::rhi::vulkan
