#include "./VksContextRequireQueueFamiliesExtension.hpp"

#include <iterator>
#include <print>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <unordered_set>

namespace strobe::renderer::vks {

static inline vk::QueueFlags toVkQueueFlags(VksQueueFlags flags) {
  vk::QueueFlags result{};

  if ((flags & VksQueueFlags::Graphics) == VksQueueFlags::Graphics) {
    result |= vk::QueueFlagBits::eGraphics;
  }
  if ((flags & VksQueueFlags::Compute) == VksQueueFlags::Compute) {
    result |= vk::QueueFlagBits::eCompute;
  }
  if ((flags & VksQueueFlags::Transfer) == VksQueueFlags::Transfer) {
    result |= vk::QueueFlagBits::eTransfer;
  }

  return result;
}

static std::optional<std::vector<uint32_t>> trySatisfyOption(
    const VksQueueSet& queueSet,
    std::span<const vk::QueueFamilyProperties> queueFamilies,
    std::span<const bool> surfaceSupport) {
  std::vector<uint32_t> assignment(queueSet.queues.size(), UINT32_MAX);

  std::function<bool(size_t)> backtrack = [&](size_t i) -> bool {
    if (i == queueSet.queues.size()) return true;

    for (uint32_t familyIndex = 0; familyIndex < queueFamilies.size(); ++familyIndex) {
      const auto& family = queueFamilies[familyIndex];
      const auto& request = queueSet.queues[i];

      const auto requiredFlags = request.flags;
      const auto avoidFlags = request.avoid;
      const auto vkFlags = toVkQueueFlags(requiredFlags);
      const auto vkAvoidFlags = toVkQueueFlags(avoidFlags);

      if ((family.queueFlags & vkFlags) != vkFlags) continue;
      if ((family.queueFlags & vkAvoidFlags) != vk::QueueFlags{}) continue;
      if ((requiredFlags & VksQueueFlags::Present) == VksQueueFlags::Present &&
          !surfaceSupport[familyIndex]) continue;

      assignment[i] = familyIndex;
      if (backtrack(i + 1)) return true;
    }

    return false;
  };

  if (backtrack(0)) return assignment;
  return std::nullopt;
}

uint64_t VksContextRequireQueueFamiliesExtension::acceptPhysicalDevice(
    vk::SurfaceKHR surface,
    const VksPhysicalDeviceInfo& physicalDeviceInfo) const {
  const auto& queueFamilyProperties =
      physicalDeviceInfo.queueFamilyProperties();
  std::span<const bool> surfaceSupport =
      physicalDeviceInfo.queueFamilySurfaceSupport(surface);

  for (size_t i = 0; i < m_options.size(); ++i) {
    if (trySatisfyOption(m_options[i], queueFamilyProperties, surfaceSupport)) {
      return m_options.size() - i;
    }
  }

  return 0;
}

void VksContextRequireQueueFamiliesExtension::appendQueueCreateInfo(
    std::vector<vk::DeviceQueueCreateInfo>& outQueueCreateInfos,
    std::vector<float>&, // unused parameter
    vk::SurfaceKHR surface,
    const VksPhysicalDeviceInfo& physicalDeviceInfo) {

  m_priorities.clear();

  const auto& queueFamilyProperties =
      physicalDeviceInfo.queueFamilyProperties();
  std::span<const bool> surfaceSupport =
      physicalDeviceInfo.queueFamilySurfaceSupport(surface);

  for (const auto& option : m_options) {
    auto assignmentOpt = trySatisfyOption(option, queueFamilyProperties, surfaceSupport);
    if (!assignmentOpt) continue;

    const auto& assignment = *assignmentOpt;
    std::unordered_map<uint32_t, std::vector<float>> familyToPriorities;
    std::unordered_map<uint32_t, size_t> familyStartOffset;

    // Pre-fill priorities and record offsets for each family
    for (size_t i = 0; i < assignment.size(); ++i) {
      uint32_t familyIndex = assignment[i];
      if (familyToPriorities.find(familyIndex) == familyToPriorities.end()) {
        familyStartOffset[familyIndex] = m_priorities.size();
        familyToPriorities[familyIndex] = {};
      }
      familyToPriorities[familyIndex].push_back(1.0f);
      m_priorities.push_back(1.0f);
    }

    for (const auto& [familyIndex, priorities] : familyToPriorities) {
      vk::DeviceQueueCreateInfo createInfo;
      createInfo.queueFamilyIndex = familyIndex;
      createInfo.queueCount = static_cast<uint32_t>(priorities.size());
      createInfo.pQueuePriorities = m_priorities.data() + familyStartOffset[familyIndex];
      outQueueCreateInfos.push_back(createInfo);
    }
    return;
  }
}

}  // namespace strobe::renderer::vks
