#include "strobe/rhi/vulkan/context/vma.hpp"
#include <atomic>
#include <stdexcept>
#include <vk_mem_alloc.h>

namespace strobe::rhi::vulkan {

class VmaDeviceMemoryTracker {
public:
  VmaDeviceMemoryTracker() noexcept
      : m_callbacks{
            .pfnAllocate = &on_allocate,
            .pfnFree = &on_free,
            .pUserData = this,
        } {}

  VmaDeviceMemoryTracker(const VmaDeviceMemoryTracker &) = delete;

  VmaDeviceMemoryTracker &operator=(const VmaDeviceMemoryTracker &) = delete;

  [[nodiscard]]
  const VmaDeviceMemoryCallbacks *callbacks() const noexcept {
    return &m_callbacks;
  }

  void reset() noexcept { m_total.store(0, std::memory_order_relaxed); }

private:
  static void VKAPI_PTR on_allocate(VmaAllocator,
                                    [[maybe_unused]] uint32_t memory_type,
                                    [[maybe_unused]] VkDeviceMemory memory,
                                    VkDeviceSize size,
                                    void *user_data) noexcept {
    auto &self = *static_cast<VmaDeviceMemoryTracker *>(user_data);
    self.m_total.fetch_add(size, std::memory_order_relaxed);
    TracyPlot("vma-device-memory", static_cast<int64_t>(self.m_total.load(
                                       std::memory_order_relaxed)));
  }

  static void VKAPI_PTR on_free(VmaAllocator,
                                [[maybe_unused]] uint32_t memory_type,
                                [[maybe_unused]] VkDeviceMemory memory,
                                VkDeviceSize size, void *user_data) noexcept {
    auto &self = *static_cast<VmaDeviceMemoryTracker *>(user_data);
    self.m_total.fetch_sub(size, std::memory_order_relaxed);
    TracyPlot("vma-device-memory", static_cast<int64_t>(self.m_total.load(
                                       std::memory_order_relaxed)));
  }

  VmaDeviceMemoryCallbacks m_callbacks{};

  std::atomic<VkDeviceSize> m_total{0};
};

#if defined(TRACY_ENABLE) && defined(STROBE_INSTRUMENT_VMA)

namespace {
VmaDeviceMemoryTracker g_tracker;
} // namespace

#endif

VmaAllocator create_vma(VkInstance instance, VkPhysicalDevice physicalDevice,
                        VkDevice device, uint32_t apiVersion,
                        const DriverAlloc *driverAlloc) {
  ZoneScopedN("context/create-vma");
  const VmaDeviceMemoryCallbacks *deviceMemoryCallbacks = nullptr;
#if defined(TRACY_ENABLE) && defined(STROBE_INSTRUMENT_VMA)
  g_tracker.reset();
  deviceMemoryCallbacks = g_tracker.callbacks();
#endif

  VmaVulkanFunctions vulkanFunctions{};
  vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo create_info{
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT,
      .physicalDevice = physicalDevice,
      .device = device,
      .preferredLargeHeapBlockSize = 0,
      .pAllocationCallbacks = driverAlloc->callbacks(),
      .pDeviceMemoryCallbacks = deviceMemoryCallbacks,
      .pHeapSizeLimit = nullptr,
      .pVulkanFunctions = &vulkanFunctions,
      .instance = instance,
      .vulkanApiVersion = apiVersion,
      .pTypeExternalMemoryHandleTypes = nullptr,
  };

  VmaAllocator allocator = VK_NULL_HANDLE;
  const VkResult result = vmaCreateAllocator(&create_info, &allocator);
  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create VMA allocator"};
  }
  return allocator;
}

} // namespace strobe::rhi::vulkan
