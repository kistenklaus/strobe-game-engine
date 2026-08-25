#pragma once

#include "strobe/rhi/types/memory_usage.hpp"
#include "strobe/rhi/vulkan/memory_usage.hpp"
#include <utility>

namespace strobe::rhi {

static inline vulkan::MemoryUsage to_vulkan_memory_usage(MemoryUsage usage) {
  switch (usage) {
  case MemoryUsage::automatic:
    return vulkan::MemoryUsage::automatic;
  case MemoryUsage::device:
    return vulkan::MemoryUsage::device;
  case MemoryUsage::mapped:
    return vulkan::MemoryUsage::mapped;
  case MemoryUsage::mapped_write_sequential:
    return vulkan::MemoryUsage::mapped_write_sequential;
  case MemoryUsage::mapped_incoherent:
    return vulkan::MemoryUsage::mapped_incoherent;
  }

  std::unreachable();
}

static inline MemoryUsage from_vulkan_memory_usage(vulkan::MemoryUsage usage) {
  switch (usage) {
  case vulkan::MemoryUsage::automatic:
    return MemoryUsage::automatic;
  case vulkan::MemoryUsage::device:
    return MemoryUsage::device;
  case vulkan::MemoryUsage::mapped:
    return MemoryUsage::mapped;
  case vulkan::MemoryUsage::mapped_write_sequential:
    return MemoryUsage::mapped_write_sequential;
  case vulkan::MemoryUsage::mapped_incoherent:
    return MemoryUsage::mapped_incoherent;
  }

  std::unreachable();
}

} // namespace strobe::rhi
