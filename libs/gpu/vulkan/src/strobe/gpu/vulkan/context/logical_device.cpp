#include "strobe/gpu/vulkan/context/logical_device.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"

namespace strobe::gpu::vulkan {

namespace details {
template <Allocator Alloc>
[[nodiscard]]
static bool
supports_extension(const Vector<DeviceExtension<Alloc>, Alloc> &extensions,
                   std::string_view required) noexcept {
  return std::ranges::any_of(
      extensions, [required](const DeviceExtension<Alloc> &extension) noexcept {
        return std::string_view{extension.name.c_str()} == required;
      });
}
} // namespace details

VkDevice
create_logical_device(VkPhysicalDevice physicalDevice,
                      const DeviceInfo<vulkan::allocator_ref> *deviceInfo,
                      span<const QueueLocation> queueLocations,
                      ContextProperties *properties,
                      const ContextCreateInfo *info, DriverAlloc *driverAlloc) {
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, 1 << 14>;

  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  struct QueueFamilyPlan {
    uint32_t family = QueueLocation::invalid;
    Vector<float, scratch_allocator_ref> priorities;
  };

  scratch_allocator scratch{};

  void *pNext = nullptr;
  VkPhysicalDeviceVulkan11Features vulkan11{};
  vulkan11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
  if (properties->api_version >= VK_API_VERSION_1_1) {
    vulkan11.pNext = pNext;
    pNext = &vulkan11;
  }

  VkPhysicalDeviceVulkan12Features vulkan12{};
  vulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  if (properties->api_version >= VK_API_VERSION_1_2) {
    vulkan12.pNext = pNext;
    pNext = &vulkan12;
  }

  VkPhysicalDeviceVulkan13Features vulkan13{};
  vulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  if (properties->api_version >= VK_API_VERSION_1_3) {
    vulkan13.pNext = pNext;
    pNext = &vulkan13;
  }

  VkPhysicalDeviceVulkan14Features vulkan14{};
  vulkan14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
  if (properties->api_version >= VK_API_VERSION_1_4) {
    vulkan14.pNext = pNext;
    pNext = &vulkan14;
  }

  Vector<const char *, scratch_allocator_ref> extensions{&scratch};

  if (info->swapchain != disable && !properties->surface) {
    throw std::runtime_error{
        "Vulkan swapchain support requires enabled surface support"};
  }
  if (info->swapchain != disable) {
    const bool supported = details::supports_extension(
        deviceInfo->supported_extensions, "VK_KHR_swapchain");
    if (supported) {
      extensions.emplace_back("VK_KHR_swapchain");
      properties->swapchain = true;
    } else if (info->swapchain == required) {
      throw std::runtime_error{"Required Vulkan device extension "
                               "VK_KHR_swapchain is not supported"};
    }
  }

  if (info->timeline_semaphore != disable &&
      deviceInfo->features.timelineSemaphore) {
    vulkan12.timelineSemaphore = true;
    extensions.emplace_back("VK_KHR_timeline_semaphore");
  }

  if (deviceInfo->features.synchronization2) {
    vulkan13.synchronization2 = true;
  }

  Vector<QueueFamilyPlan, scratch_allocator_ref> queueFamilyPlans{&scratch};
  queueFamilyPlans.reserve(queueLocations.size());
  for (uint32_t i = 0; i < queueLocations.size(); ++i) {
    const QueueLocation location = queueLocations[i];
    if (!location) {
      continue;
    }
    const float priority = info->pQueues[i].priority;
    if (priority < 0.0f || priority > 1.0f) {
      throw std::runtime_error{"Vulkan queue priority must be between 0 and 1"};
    }
    auto it = std::ranges::find_if(
        queueFamilyPlans,
        [family = location.family](const QueueFamilyPlan &plan) {
          return plan.family == family;
        });
    if (it == queueFamilyPlans.end()) {
      queueFamilyPlans.push_back(QueueFamilyPlan{
          .family = location.family,
          .priorities = Vector<float, scratch_allocator_ref>{&scratch},
      });
      it = queueFamilyPlans.end() - 1;
    }
    if (it->priorities.size() <= location.index) {
      it->priorities.resize(location.index + 1, 1.0f);
    }
    it->priorities[location.index] = priority;
  }

  Vector<VkDeviceQueueCreateInfo, scratch_allocator_ref> queueCreateInfos{
      &scratch};
  queueCreateInfos.reserve(queueFamilyPlans.size());
  for (const QueueFamilyPlan &plan : queueFamilyPlans) {
    queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = plan.family,
        .queueCount = static_cast<uint32_t>(plan.priorities.size()),
        .pQueuePriorities = plan.priorities.data(),
    });
  }

  if (queueCreateInfos.empty()) {
    throw std::runtime_error{"Cannot create a Vulkan device without queues"};
  }

  VkPhysicalDeviceFeatures2 features2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = pNext,
      .features = {},
  };

  pNext = &features2;

  VkDeviceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = pNext,
      .flags = 0,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
      .pEnabledFeatures = nullptr,
  };

  VkDevice device = VK_NULL_HANDLE;

  const VkResult result = vkCreateDevice(physicalDevice, &createInfo,
                                         driverAlloc->callbacks(), &device);

  if (result != VK_SUCCESS) {
    throw std::runtime_error{"Failed to create logical device"};
  }

  return device;
}
} // namespace strobe::gpu::vulkan
