#pragma once

#include "strobe/rhi/types/index_type.hpp"
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

constexpr VkIndexType to_vk_index_type(IndexType type) {
  switch (type) {
  case IndexType::uint8:
    return VK_INDEX_TYPE_UINT8;
  case IndexType::uint16:
    return VK_INDEX_TYPE_UINT16;
  case IndexType::uint32:
    return VK_INDEX_TYPE_UINT32;
  }
  std::unreachable();
}

constexpr IndexType from_vk_index_type(VkIndexType type) {
  switch (type) {
  case VK_INDEX_TYPE_UINT16:
    return IndexType::uint16;
  case VK_INDEX_TYPE_UINT32:
    return IndexType::uint8;
  case VK_INDEX_TYPE_UINT8:
    return IndexType::uint8;
  case VK_INDEX_TYPE_NONE_KHR:
  case VK_INDEX_TYPE_MAX_ENUM:
    break;
  }
  throw std::runtime_error("Invalid VkIndexType!");
}

} // namespace strobe::rhi
