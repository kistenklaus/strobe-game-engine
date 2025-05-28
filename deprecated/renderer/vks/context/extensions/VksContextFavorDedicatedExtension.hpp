#pragma once

#include "renderer/vks/context/VksContextExtension.hpp"

namespace strobe::renderer::vks {

class VksContextFavorDedicatedExtension : public VksContextExtension {
 public:
  VksContextFavorDedicatedExtension(uint64_t score = 1) : m_score(score) {}

  uint64_t acceptPhysicalDevice(
      [[maybe_unused]] vk::SurfaceKHR surface, const VksPhysicalDeviceInfo& physicalDeviceInfo) const final override {
    return physicalDeviceInfo.properties().deviceType ==
                   vk::PhysicalDeviceType::eDiscreteGpu
               ? m_score
               : 0;
  }

 private:
  uint64_t m_score;
};

}  // namespace strobe::renderer::vks
