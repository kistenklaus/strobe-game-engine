#pragma once

#include "renderer/vks/context/VksContextExtension.hpp"
#include "renderer/vks/context/VksQueueInfo.hpp"

namespace strobe::renderer::vks {

enum class VksQueueFlags {
  None = 0x0,
  Graphics = 0x1,
  Compute = 0x2,
  Transfer = 0x4,
  Present = 0x8,
};

// Enable bitwise ops
inline VksQueueFlags operator|(VksQueueFlags lhs, VksQueueFlags rhs) {
  return static_cast<VksQueueFlags>(static_cast<uint32_t>(lhs) |
                                    static_cast<uint32_t>(rhs));
}

inline VksQueueFlags operator&(VksQueueFlags lhs, VksQueueFlags rhs) {
  return static_cast<VksQueueFlags>(static_cast<uint32_t>(lhs) &
                                    static_cast<uint32_t>(rhs));
}

inline VksQueueFlags operator^(VksQueueFlags lhs, VksQueueFlags rhs) {
  return static_cast<VksQueueFlags>(static_cast<uint32_t>(lhs) ^
                                    static_cast<uint32_t>(rhs));
}

inline VksQueueFlags operator~(VksQueueFlags value) {
  return static_cast<VksQueueFlags>(~static_cast<uint32_t>(value));
}

inline VksQueueFlags& operator|=(VksQueueFlags& lhs, VksQueueFlags rhs) {
  lhs = lhs | rhs;
  return lhs;
}

inline VksQueueFlags& operator&=(VksQueueFlags& lhs, VksQueueFlags rhs) {
  lhs = lhs & rhs;
  return lhs;
}

inline VksQueueFlags& operator^=(VksQueueFlags& lhs, VksQueueFlags rhs) {
  lhs = lhs ^ rhs;
  return lhs;
}

struct VksQueueRequest {
  VksQueueFlags flags; // Mandatory queue flags
  VksQueueFlags avoid; // Queue flags which have to be avoided!
};

struct VksQueueSet {
  std::vector<VksQueueRequest> queues;
};

// tried to select queue family indices, such that the minimum amount of flags
// are set.
class VksContextRequireQueueFamiliesExtension : public VksContextExtension {
 public:
  explicit VksContextRequireQueueFamiliesExtension(std::vector<VksQueueSet> options)
      : m_options(options) {
  }

  uint64_t acceptPhysicalDevice(
      const vk::SurfaceKHR surface,
      const VksPhysicalDeviceInfo& physicalDeviceInfo) const final override;

  void appendQueueCreateInfo(
      std::vector<vk::DeviceQueueCreateInfo>& createInfos,
      std::vector<float>& queuePriorities, vk::SurfaceKHR surface,
      const VksPhysicalDeviceInfo& physicalDeviceInfo) final override;

 private:
  std::optional<std::vector<uint32_t>> findBestQueueFamilyAssignment(
      std::span<const bool> queueFamilyPresentSupport,
      std::span<const vk::QueueFamilyProperties> queueFamilyProperties) const;

  std::vector<VksQueueSet> m_options;
  std::vector<float> m_priorities;
};

}  // namespace strobe::renderer::vks
