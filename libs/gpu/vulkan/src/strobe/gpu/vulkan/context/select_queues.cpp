#include "strobe/gpu/vulkan/context/select_queues.hpp"

#include <bit>
#include <limits>
#include <stdexcept>

namespace strobe::gpu::vulkan {

namespace details {

[[nodiscard]]
constexpr bool
queue_family_matches(const QueueFamilyProperties &family,
                     const QueueDescription &description) noexcept {
  const VkQueueFlags flags = family.queueFlags;

  if (family.queueCount == 0) {
    return false;
  }

  if ((flags & description.require) != description.require) {
    return false;
  }

  if ((flags & description.exclude) != 0) {
    return false;
  }

  /*
   * Presentation support is relative to the surface that was supplied when
   * DeviceInfo was queried.
   *
   * If no surface exists, QueueFamilyProperties::presentationSupport is false
   * for every family, so `present == required` naturally cannot match.
   */
  if (description.present == required && !family.presentationSupport) {
    return false;
  }

  return true;
}

[[nodiscard]]
constexpr int queue_family_score(const QueueFamilyProperties &family,
                                 const QueueDescription &description,
                                 uint32_t used_queue_count) noexcept {
  const VkQueueFlags preferred =
      description.prefer & ~description.require & ~description.exclude;

  const VkQueueFlags relevant = description.require | preferred;

  const uint32_t preferred_count =
      std::popcount(static_cast<uint32_t>(family.queueFlags & preferred));

  const uint32_t excess_count =
      std::popcount(static_cast<uint32_t>(family.queueFlags & ~relevant));

  const uint32_t remaining_count = family.queueCount - used_queue_count;

  /*
   * Prefer:
   *
   * 1. More explicitly preferred queue capabilities.
   * 2. Presentation support when it was requested as optional.
   * 3. Fewer excess capabilities.
   * 4. Families with more unassigned queues.
   */
  int score = static_cast<int>(preferred_count) * 1'000 -
              static_cast<int>(excess_count) * 100 +
              static_cast<int>(remaining_count);

  if (description.present == optional && family.presentationSupport) {
    score += 1'000;
  }

  return score;
}

} // namespace details

Vector<QueueLocation, vulkan::allocator_ref>
select_queues(const DeviceInfo<vulkan::allocator_ref> *device_info,
              const ContextCreateInfo *info,
              const vulkan::allocator_ref &alloc) {
  assert(device_info != nullptr);
  assert(info != nullptr);
  assert(info->queue_count == 0 || info->pQueues != nullptr);

  const auto &families = device_info->queue_family_properties;

  Vector<QueueLocation, vulkan::allocator_ref> locations{alloc};

  locations.resize(info->queue_count, QueueLocation{});

  /*
   * Since queues within a family are assigned consecutively,
   * this is also the next unused queue index in that family.
   */
  Vector<uint32_t, vulkan::allocator_ref> used_queue_counts{alloc};

  used_queue_counts.resize(families.size(), 0);

  const auto assign = [&](uint32_t description_index) -> bool {
    const QueueDescription &description = info->pQueues[description_index];

    uint32_t best_family = QueueLocation::invalid;

    int best_score = std::numeric_limits<int>::min();

    for (uint32_t family_index = 0; family_index < families.size();
         ++family_index) {
      const QueueFamilyProperties &family = families[family_index];

      const uint32_t used_count = used_queue_counts[family_index];

      /*
       * Every assignment receives a distinct queue index.
       */
      if (used_count >= family.queueCount) {
        continue;
      }

      if (!details::queue_family_matches(family, description)) {
        continue;
      }

      const int score =
          details::queue_family_score(family, description, used_count);

      if (score > best_score) {
        best_score = score;
        best_family = family_index;
      }
    }

    if (best_family == QueueLocation::invalid) {
      return false;
    }

    const uint32_t queue_index = used_queue_counts[best_family]++;

    locations[description_index] = {
        .family = best_family,
        .index = queue_index,
    };

    return true;
  };

  /*
   * Required descriptions consume queue slots before optional
   * descriptions.
   */
  for (uint32_t i = 0; i < info->queue_count; ++i) {
    const QueueDescription &description = info->pQueues[i];

    if (description.available != required) {
      continue;
    }

    if (!assign(i)) {
      throw std::runtime_error("Failed to select queues");
    }
  }

  /*
   * Optional descriptions are assigned only when possible.
   *
   * If an optional queue cannot be assigned, its QueueLocation remains
   * invalid.
   */
  for (uint32_t i = 0; i < info->queue_count; ++i) {
    const QueueDescription &description = info->pQueues[i];

    if (description.available != optional) {
      continue;
    }

    assign(i);
  }

  return locations;
}

} // namespace strobe::gpu::vulkan
