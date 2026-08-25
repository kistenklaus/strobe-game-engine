#include "strobe/gpu/vulkan/context/select_physical_device.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/gpu/vulkan/context/context_properties.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"
#include "strobe/gpu/vulkan/device_info/device_extensions.hpp"
#include "strobe/gpu/vulkan/device_info/device_features.hpp"
#include "strobe/gpu/vulkan/device_info/device_info.hpp"
#include "strobe/gpu/vulkan/device_info/memory_properties.hpp"
#include "strobe/gpu/vulkan/device_info/queue_family_properties.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vulkan/vulkan_core.h>

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

template <Allocator Alloc>
[[nodiscard]]
VkDeviceSize
device_local_memory(const MemoryProperties<Alloc> &properties) noexcept {
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

  /*
   * If no surface was supplied to DeviceInfo::query(),
   * presentationSupport is false for every queue family.
   *
   * Therefore a queue description with present == required
   * automatically makes the device unsuitable in that case.
   */
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

  /*
   * Presentation support acts like another preferred capability
   * when it is optional.
   *
   * When presentation is required it is already enforced by
   * queue_family_matches().
   *
   * When presentation is disabled it has no effect.
   */
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

    /*
     * Every requested QueueDescription corresponds to one actual queue.
     *
     * A queue family can only contribute queueCount queues in total.
     */
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

template <Allocator Alloc>
[[nodiscard]]
double score_queue_descriptions(span<const QueueFamilyProperties> families,
                                span<const QueueDescription> descriptions,
                                const Alloc &alloc) {
  Vector<std::uint32_t, Alloc> used_counts{alloc};

  used_counts.resize(families.size(), 0);

  double score = 0.0;

  /*
   * Required queue descriptions must all be satisfiable.
   *
   * If even one cannot be assigned to a suitable queue family,
   * the physical device is unsuitable.
   */
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

  /*
   * Optional queue descriptions improve the score if they can be
   * satisfied, but do not make the device unsuitable if they cannot.
   */
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
                        const ContextCreateInfo *info, uint32_t apiVersion) {
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, 1 << 14>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};

  /*
   * DeviceInfo is the single source of truth for all queried physical-device
   * capabilities, including presentation support relative to `surface`.
   */
  const DeviceInfo deviceInfo = DeviceInfo<scratch_allocator_ref>::query(
      instance, device, apiVersion, &scratch);

  constexpr double unsuitable = -std::numeric_limits<double>::infinity();

  /*
   * This should be the Vulkan version actually usable with this device.
   */
  const uint32_t usableApiVersion =
      std::min(apiVersion, deviceInfo.properties.apiVersion);

  if (usableApiVersion < VK_API_VERSION_1_1) {
    return unsuitable;
  }

  /*
   * Device-extension requirements.
   */
  const bool swapchainSupported = supports_extension(
      deviceInfo.supported_extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  const bool swapchainMaintenance1Supported =
      supports_extension(deviceInfo.supported_extensions,
                         VK_KHR_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

  /*
   * Device-feature requirements.
   */
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

  double score = device_type_score(deviceInfo.properties.deviceType);

  /*
   * Queue requirements.
   *
   * Presentation requirements are evaluated here as part of the individual
   * QueueDescription. There is deliberately no separate device-wide
   * presentation check.
   */
  if (info != nullptr && info->queue_count > 0) {
    if (info->pQueues == nullptr) {
      return unsuitable;
    }

    const span<const QueueDescription> descriptions{
        info->pQueues,
        info->queue_count,
    };

    const double queueScore =
        score_queue_descriptions(deviceInfo.queue_family_properties,
                                 descriptions, scratch_allocator_ref{&scratch});

    if (!std::isfinite(queueScore)) {
      return unsuitable;
    }

    score += queueScore;
  }

  /*
   * Device-local memory is useful as a weak device-quality heuristic.
   *
   * Cap the contribution to avoid VRAM size dominating the device score.
   */
  constexpr VkDeviceSize gib = VkDeviceSize{1} << 30;

  const VkDeviceSize localMemory =
      device_local_memory(deviceInfo.memory_properties);

  const double localMemoryGiB =
      static_cast<double>(localMemory) / static_cast<double>(gib);

  score += std::min(localMemoryGiB, 32.0) * 25.0;

  /*
   * Explicitly requested optional functionality carries more weight than
   * arbitrary generally useful device features.
   */
  if (info != nullptr && info->swapchain == optional && swapchainSupported &&
      swapchainMaintenance1Supported && swapchainMaintenance1FeatureSupported) {
    score += 100.0;
  }

  if (info != nullptr && info->timeline_semaphore == optional &&
      timelineSemaphoreSupported) {
    score += 100.0;
  }

  /*
   * Generic device-quality heuristics.
   */
  score += deviceInfo.features.samplerAnisotropy ? 25.0 : 0.0;
  score += deviceInfo.features.multiDrawIndirect ? 25.0 : 0.0;
  score += deviceInfo.features.drawIndirectFirstInstance ? 25.0 : 0.0;
  score += deviceInfo.features.shaderInt64 ? 10.0 : 0.0;
  score += deviceInfo.features.fragmentStoresAndAtomics ? 10.0 : 0.0;
  score += deviceInfo.features.vertexPipelineStoresAndAtomics ? 10.0 : 0.0;

  return score;
}

} // namespace details

VkPhysicalDevice select_physical_device(VkInstance instance,
                                        const ContextCreateInfo *info,
                                        const ContextProperties *props) {
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, 1 << 14>;

  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};

  Vector<VkPhysicalDevice, scratch_allocator_ref> devices{&scratch};

  std::uint32_t count = 0;

  VkResult result = vkEnumeratePhysicalDevices(instance, &count, nullptr);

  if (result != VK_SUCCESS || count == 0) {
    throw std::runtime_error("Failed to enumerate physical devices");
  }

  /*
   * Physical-device enumeration may return VK_INCOMPLETE if the device list
   * changes between the count and data queries.
   */
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

  VkPhysicalDevice selected = VK_NULL_HANDLE;

  double best_score = -std::numeric_limits<double>::infinity();

  for (const auto &device : devices) {
    const double score =
        details::get_device_score(instance, device, info, props->api_version);

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

} // namespace strobe::gpu::vulkan
