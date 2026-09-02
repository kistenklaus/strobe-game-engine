#include "strobe/rhi/vulkan/context/select_physical_device.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/vulkan/context/context_properties.hpp"
#include "strobe/rhi/vulkan/context/create_info.hpp"
#include "strobe/rhi/vulkan/device_info/device_extensions.hpp"
#include "strobe/rhi/vulkan/device_info/device_features.hpp"
#include "strobe/rhi/vulkan/device_info/device_info.hpp"
#include "strobe/rhi/vulkan/device_info/memory_properties.hpp"
#include "strobe/rhi/vulkan/device_info/queue_family_properties.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <concepts>
#include <limits>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

namespace details {

[[nodiscard]]
VkDeviceSize device_local_memory(const MemoryProperties &properties) noexcept {
  VkDeviceSize result = 0;
  for (std::uint32_t i = 0; i < properties.memoryHeaps.size(); ++i) {
    const auto &heap = properties.memoryHeaps[i];
    if ((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0) {
      result += heap.size;
    }
  }
  return result;
}

[[nodiscard]]
constexpr double device_type_score(VkPhysicalDeviceType type) noexcept {
  switch (type) {
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    return 10'000.0;

  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    return 7'500.0;

  case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    return 5'000.0;

  case VK_PHYSICAL_DEVICE_TYPE_CPU:
    return 1'000.0;

  case VK_PHYSICAL_DEVICE_TYPE_OTHER:
  default:
    return 2'500.0;
  }
}

[[nodiscard]]
constexpr bool
queue_family_matches(const QueueFamilyProperties &family,
                     const QueueDescription &description) noexcept {
  if (family.queueCount == 0) {
    return false;
  }

  if ((family.queueFlags & description.require) != description.require) {
    return false;
  }

  if ((family.queueFlags & description.exclude) != 0) {
    return false;
  }
  if (description.present == required && !family.presentationSupport) {
    return false;
  }
  return true;
}

[[nodiscard]]
constexpr double
queue_family_score(const QueueFamilyProperties &family,
                   const QueueDescription &description) noexcept {
  const VkQueueFlags preferred =
      description.prefer & ~description.require & ~description.exclude;

  const VkQueueFlags relevant = description.require | preferred;

  const auto preferred_count =
      std::popcount(static_cast<std::uint32_t>(family.queueFlags & preferred));

  const auto excess_count =
      std::popcount(static_cast<std::uint32_t>(family.queueFlags & ~relevant));

  double score = static_cast<double>(preferred_count) * 100.0 -
                 static_cast<double>(excess_count) * 10.0;

  if (description.present == optional && family.presentationSupport) {
    score += 100.0;
  }

  return score;
}

template <Allocator Alloc>
[[nodiscard]]
bool assign_queue(span<const QueueFamilyProperties> families,
                  const QueueDescription &description,
                  Vector<std::uint32_t, Alloc> &used_counts, double &score) {
  std::uint32_t best_family = std::numeric_limits<std::uint32_t>::max();
  double best_score = -std::numeric_limits<double>::infinity();
  for (std::uint32_t family_index = 0; family_index < families.size();
       ++family_index) {
    const auto &family = families[family_index];
    if (used_counts[family_index] >= family.queueCount) {
      continue;
    }
    if (!queue_family_matches(family, description)) {
      continue;
    }
    const double candidate_score = queue_family_score(family, description);
    if (candidate_score > best_score) {
      best_score = candidate_score;
      best_family = family_index;
    }
  }
  if (best_family == std::numeric_limits<std::uint32_t>::max()) {
    return false;
  }
  ++used_counts[best_family];
  score = best_score;
  return true;
}

[[nodiscard]]
double score_queue_descriptions(span<const QueueFamilyProperties> families,
                                span<const QueueDescription> descriptions,
                                const strobe::rhi::allocator_ref &alloc) {
  Vector<std::uint32_t, strobe::rhi::allocator_ref> used_counts{alloc};
  used_counts.resize(families.size(), 0);
  double score = 0.0;
  for (const auto &description : descriptions) {
    if (description.available != required) {
      continue;
    }
    double assignment_score = 0.0;
    if (!assign_queue(families, description, used_counts, assignment_score)) {
      return -std::numeric_limits<double>::infinity();
    }
    score += 250.0 + assignment_score;
  }
  for (const auto &description : descriptions) {
    if (description.available != optional) {
      continue;
    }
    double assignment_score = 0.0;
    if (assign_queue(families, description, used_counts, assignment_score)) {
      score += 100.0 + assignment_score;
    }
  }
  return score;
}

[[nodiscard]]
double get_device_score(VkInstance instance, VkPhysicalDevice device,
                        const ContextCreateInfo *info, uint32_t apiVersion,
                        const strobe::rhi::allocator_ref &alloc) {
  const DeviceInfo deviceInfo =
      DeviceInfo::query(instance, device, apiVersion, alloc);
  constexpr double unsuitable = -std::numeric_limits<double>::infinity();

  const uint32_t usableApiVersion =
      std::min(apiVersion, deviceInfo.properties.apiVersion);

  if (usableApiVersion < VK_API_VERSION_1_1) {
    return unsuitable;
  }

  const bool swapchainSupported = supports_extension(
      deviceInfo.supported_extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  const bool swapchainMaintenance1Supported =
      supports_extension(deviceInfo.supported_extensions,
                         VK_KHR_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

  const bool timelineSemaphoreSupported = deviceInfo.features.timelineSemaphore;
  const bool swapchainMaintenance1FeatureSupported =
      deviceInfo.features.swapchainMaintenance1;
  if (info != nullptr) {
    if (info->swapchain == required) {
      if (!swapchainSupported) {
        return unsuitable;
      }
      if (!swapchainMaintenance1Supported) {
        return unsuitable;
      }
      if (!swapchainMaintenance1FeatureSupported) {
        return unsuitable;
      }
    }

    if (info->timeline_semaphore == required && !timelineSemaphoreSupported) {
      return unsuitable;
    }
  }

  if (!deviceInfo.features.shaderObjects) {
    return unsuitable;
  }

  if (!deviceInfo.features.dynamicRendering) {
    return unsuitable;
  }

  if (info->hostQueryReset == required && !deviceInfo.features.hostQueryReset) {
    return unsuitable;
  }

  if (info->calibratedTimestamps == required &&
      !deviceInfo.features.calibratedTimestamps) {
    return unsuitable;
  }

  if (info->bufferDeviceAddress == required &&
      !deviceInfo.features.bufferDeviceAddress) {
    return unsuitable;
  }

  if (info->deferredHostOperations == required &&
      !deviceInfo.features.deferredHostOperations) {
    return unsuitable;
  }
  if (info->accelerationStructure == required &&
      !deviceInfo.features.accelerationStructure) {
    return unsuitable;
  }
  if (info->raytracingPipeline == required &&
      !deviceInfo.features.rayTracingPipeline) {
    return unsuitable;
  }

  if (info->rayQuery == required && !deviceInfo.features.rayQuery) {
    return unsuitable;
  }

  if (info->descriptorHeap == required && !deviceInfo.features.descriptorHeap) {
    return unsuitable;
  }

  if (!deviceInfo.features.maintenance9) {
    return unsuitable;
  }

  double score = device_type_score(deviceInfo.properties.deviceType);

  if (info != nullptr && info->queue_count > 0) {
    if (info->pQueues == nullptr) {
      return unsuitable;
    }
    const span<const QueueDescription> descriptions{
        info->pQueues,
        info->queue_count,
    };
    const double queueScore = score_queue_descriptions(
        deviceInfo.queue_family_properties, descriptions, alloc);
    if (!std::isfinite(queueScore)) {
      return unsuitable;
    }
    score += queueScore;
  }

  constexpr VkDeviceSize gib = VkDeviceSize{1} << 30;
  const VkDeviceSize localMemory =
      device_local_memory(deviceInfo.memory_properties);
  const double localMemoryGiB =
      static_cast<double>(localMemory) / static_cast<double>(gib);
  score += std::min(localMemoryGiB, 32.0) * 25.0;

  if (info != nullptr && info->swapchain == optional && swapchainSupported &&
      swapchainMaintenance1Supported && swapchainMaintenance1FeatureSupported) {
    score += 100.0;
  }

  if (info != nullptr && info->timeline_semaphore == optional &&
      timelineSemaphoreSupported) {
    score += 100.0;
  }

  score += deviceInfo.features.samplerAnisotropy ? 25.0 : 0.0;
  score += deviceInfo.features.multiDrawIndirect ? 25.0 : 0.0;
  score += deviceInfo.features.drawIndirectFirstInstance ? 25.0 : 0.0;
  score += deviceInfo.features.shaderInt64 ? 10.0 : 0.0;
  score += deviceInfo.features.fragmentStoresAndAtomics ? 10.0 : 0.0;
  score += deviceInfo.features.vertexPipelineStoresAndAtomics ? 10.0 : 0.0;

  return score;
}

} // namespace details

VkPhysicalDevice
select_physical_device(VkInstance instance, const ContextCreateInfo *info,
                       const ContextProperties *props,
                       const strobe::rhi::allocator_ref alloc) {
  ZoneScopedN("context/select-physical-device");
  using scratch_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 14>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};
  Vector<VkPhysicalDevice, scratch_allocator_ref> devices{&scratch};
  std::uint32_t count = 0;
  {
    ZoneScopedN("vkEnumeratePhysicalDevices");
    VkResult result = vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (result != VK_SUCCESS || count == 0) {
      throw std::runtime_error("Failed to enumerate physical devices");
    }
    while (true) {
      devices.resize(static_cast<size_t>(count));
      result = vkEnumeratePhysicalDevices(instance, &count, devices.data());
      devices.resize(count);
      if (result == VK_SUCCESS) {
        break;
      }
      if (result != VK_INCOMPLETE) {
        throw std::runtime_error("Failed to enumerate physical devices");
      }
    }
  }

  VkPhysicalDevice selected = VK_NULL_HANDLE;
  double best_score = -std::numeric_limits<double>::infinity();
  for (const auto &device : devices) {
    const double score = details::get_device_score(instance, device, info,
                                                   props->api_version, alloc);
    if (score > best_score) {
      best_score = score;
      selected = device;
    }
  }
  if (selected == VK_NULL_HANDLE) {
    throw std::runtime_error("Failed to select suitable device");
  }
  return selected;
}

} // namespace strobe::rhi::vulkan
