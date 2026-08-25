#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/vulkan/context/context_properties.hpp"
#include "strobe/rhi/vulkan/context/create_info.hpp"
#include "strobe/rhi/vulkan/context/driver_alloc.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include "strobe/rhi/vulkan/context/select_queues.hpp"
#include "strobe/rhi/vulkan/queue_type.hpp"

#include <stdexcept>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

class Context {
public:
  Context(const ContextCreateInfo &info,
          const strobe::rhi::allocator_ref &alloc) noexcept;

  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;
  Context(Context &&) = delete;
  Context &operator=(Context &&) = delete;
  ~Context() noexcept;

  VkInstance instance() const noexcept {
    assert(m_instance != VK_NULL_HANDLE);
    return m_instance;
  }
  VkPhysicalDevice physicalDevice() const noexcept {
    assert(m_physicalDevice != VK_NULL_HANDLE);
    return m_physicalDevice;
  }
  VkDevice device() const noexcept {
    assert(m_device != VK_NULL_HANDLE);
    return m_device;
  }
  const ContextProperties &properties() const noexcept { return m_props; }
  const DeviceInfo &deviceInfo() const noexcept { return m_deviceInfo; }

  // may return VK_NULL_HANDLE!
  Queue queue(uint32_t index) const noexcept {
    assert(index < m_queues.size());
    return Queue{
        .handle = m_queues[index],
        .family = m_queueLocations[index].family,
    };
  }

  inline __attribute((always_inline)) const VkAllocationCallbacks *
  driver_alloc() const noexcept {
    return m_driverAlloc.callbacks();
  }

  VmaAllocator vma() const noexcept { return m_vma; }

  const PNF_Functions *pnf() const noexcept { return &m_pnf; }

  void wait_idle() const {
    VkResult result = vkDeviceWaitIdle(m_device);
    if (result != VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to wait for device idle : device timed out.");
    }
  }

private:
  [[no_unique_address]] strobe::rhi::allocator_ref m_alloc;
  DriverAlloc m_driverAlloc;
  ContextProperties m_props;
  VkInstance m_instance;
  VkDebugUtilsMessengerEXT m_messenger;

  VkPhysicalDevice m_physicalDevice;

  DeviceInfo m_deviceInfo;

  Vector<QueueLocation, strobe::rhi::allocator_ref> m_queueLocations;

  VkDevice m_device;
  Vector<VkQueue, strobe::rhi::allocator_ref> m_queues;
  VmaAllocator m_vma;

  PNF_Functions m_pnf;
};

} // namespace strobe::rhi::vulkan
