#pragma once

#include "strobe/rhi/types/buffer_usage.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

static inline VkBufferUsageFlags2 to_vk_buffer_usage(BufferUsage usage) {
  VkBufferUsageFlags2 result = 0;

  if ((usage & BufferUsage::transfer_src) != 0) {
    result |= VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT;
  }

  if ((usage & BufferUsage::transfer_dst) != 0) {
    result |= VK_BUFFER_USAGE_2_TRANSFER_DST_BIT;
  }

  if ((usage & BufferUsage::uniform_texel) != 0) {
    result |= VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT;
  }

  if ((usage & BufferUsage::storage_texel) != 0) {
    result |= VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT;
  }

  if ((usage & BufferUsage::uniform) != 0) {
    result |= VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT;
  }

  if ((usage & BufferUsage::storage) != 0) {
    result |= VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT;
  }

  if ((usage & BufferUsage::index) != 0) {
    result |= VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT;
  }

  if ((usage & BufferUsage::vertex) != 0) {
    result |= VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT;
  }

  if ((usage & BufferUsage::indirect) != 0) {
    result |= VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT;
  }

  if ((usage & BufferUsage::shader_device_address) != 0) {
    result |= VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
  }

  if ((usage & BufferUsage::shader_binding_table) != 0) {
    result |= VK_BUFFER_USAGE_2_SHADER_BINDING_TABLE_BIT_KHR;
  }

  if ((usage & BufferUsage::acceleration_structure_build_input) != 0) {
    result |=
        VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
  }

  if ((usage & BufferUsage::acceleration_structure_storage) != 0) {
    result |= VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
  }

  if ((usage & BufferUsage::descriptor_heap) != 0) {
    result |= VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT;
  }

  return result;
}

static inline BufferUsage from_vk_buffer_usage(VkBufferUsageFlags2 usage) {
  BufferUsage result = BufferUsage::none;

  if ((usage & VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT) != 0) {
    result |= BufferUsage::transfer_src;
  }

  if ((usage & VK_BUFFER_USAGE_2_TRANSFER_DST_BIT) != 0) {
    result |= BufferUsage::transfer_dst;
  }

  if ((usage & VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT) != 0) {
    result |= BufferUsage::uniform_texel;
  }

  if ((usage & VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT) != 0) {
    result |= BufferUsage::storage_texel;
  }

  if ((usage & VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT) != 0) {
    result |= BufferUsage::uniform;
  }

  if ((usage & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) != 0) {
    result |= BufferUsage::storage;
  }

  if ((usage & VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT) != 0) {
    result |= BufferUsage::index;
  }

  if ((usage & VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT) != 0) {
    result |= BufferUsage::vertex;
  }

  if ((usage & VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT) != 0) {
    result |= BufferUsage::indirect;
  }

  if ((usage & VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT) != 0) {
    result |= BufferUsage::shader_device_address;
  }

  if ((usage & VK_BUFFER_USAGE_2_SHADER_BINDING_TABLE_BIT_KHR) != 0) {
    result |= BufferUsage::shader_binding_table;
  }

  if ((usage &
       VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) !=
      0) {
    result |= BufferUsage::acceleration_structure_build_input;
  }

  if ((usage & VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR) != 0) {
    result |= BufferUsage::acceleration_structure_storage;
  }

  if ((usage & VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT) != 0) {
    result |= BufferUsage::descriptor_heap;
  }

  return result;
}

} // namespace strobe::rhi
