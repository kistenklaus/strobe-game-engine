#pragma once

#include "strobe/core/containers/span.hpp"
#include <cstdint>

namespace strobe::gpu::vulkan {

struct SpecializationEntry {
  uint32_t id = 0;
  uint32_t offset = 0;
  uint32_t size = 0;
};

struct SpecializationInfo {
  span<const SpecializationEntry> entries = {};
  span<const std::byte> data;
};

} // namespace strobe::gpu::vulkan
