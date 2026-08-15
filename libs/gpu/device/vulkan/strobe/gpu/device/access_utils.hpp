#pragma once

#include "strobe/gpu/device/access.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

static inline VkAccessFlags2 to_vk_access(Access flags) {
  VkAccessFlags2 result = 0;

  if ((flags & Access::indirect_command_read) != 0) {
    result |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
  }

  if ((flags & Access::index_read) != 0) {
    result |= VK_ACCESS_2_INDEX_READ_BIT;
  }

  if ((flags & Access::vertex_attribute_read) != 0) {
    result |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
  }

  if ((flags & Access::uniform_read) != 0) {
    result |= VK_ACCESS_2_UNIFORM_READ_BIT;
  }

  if ((flags & Access::shader_sampled_read) != 0) {
    result |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
  }

  if ((flags & Access::shader_storage_read) != 0) {
    result |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
  }

  if ((flags & Access::shader_storage_write) != 0) {
    result |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
  }

  if ((flags & Access::color_attachment_read) != 0) {
    result |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
  }

  if ((flags & Access::color_attachment_write) != 0) {
    result |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  }

  if ((flags & Access::depth_stencil_attachment_read) != 0) {
    result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  }

  if ((flags & Access::depth_stencil_attachment_write) != 0) {
    result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  }

  if ((flags & Access::transfer_read) != 0) {
    result |= VK_ACCESS_2_TRANSFER_READ_BIT;
  }

  if ((flags & Access::transfer_write) != 0) {
    result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
  }

  if ((flags & Access::host_read) != 0) {
    result |= VK_ACCESS_2_HOST_READ_BIT;
  }

  if ((flags & Access::host_write) != 0) {
    result |= VK_ACCESS_2_HOST_WRITE_BIT;
  }

  if ((flags & Access::memory_read) != 0) {
    result |= VK_ACCESS_2_MEMORY_READ_BIT;
  }

  if ((flags & Access::memory_write) != 0) {
    result |= VK_ACCESS_2_MEMORY_WRITE_BIT;
  }

  return result;
}

static inline Access from_vk_access(VkAccessFlags2 flags) {
  Access result = Access::none;

  if ((flags & VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT) != 0) {
    result |= Access::indirect_command_read;
  }

  if ((flags & VK_ACCESS_2_INDEX_READ_BIT) != 0) {
    result |= Access::index_read;
  }

  if ((flags & VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT) != 0) {
    result |= Access::vertex_attribute_read;
  }

  if ((flags & VK_ACCESS_2_UNIFORM_READ_BIT) != 0) {
    result |= Access::uniform_read;
  }

  if ((flags & VK_ACCESS_2_SHADER_SAMPLED_READ_BIT) != 0) {
    result |= Access::shader_sampled_read;
  }

  if ((flags & VK_ACCESS_2_SHADER_STORAGE_READ_BIT) != 0) {
    result |= Access::shader_storage_read;
  }

  if ((flags & VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT) != 0) {
    result |= Access::shader_storage_write;
  }

  if ((flags & VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT) != 0) {
    result |= Access::color_attachment_read;
  }

  if ((flags & VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT) != 0) {
    result |= Access::color_attachment_write;
  }

  if ((flags & VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT) != 0) {
    result |= Access::depth_stencil_attachment_read;
  }

  if ((flags & VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT) != 0) {
    result |= Access::depth_stencil_attachment_write;
  }

  if ((flags & VK_ACCESS_2_TRANSFER_READ_BIT) != 0) {
    result |= Access::transfer_read;
  }

  if ((flags & VK_ACCESS_2_TRANSFER_WRITE_BIT) != 0) {
    result |= Access::transfer_write;
  }

  if ((flags & VK_ACCESS_2_HOST_READ_BIT) != 0) {
    result |= Access::host_read;
  }

  if ((flags & VK_ACCESS_2_HOST_WRITE_BIT) != 0) {
    result |= Access::host_write;
  }

  if ((flags & VK_ACCESS_2_MEMORY_READ_BIT) != 0) {
    result |= Access::memory_read;
  }

  if ((flags & VK_ACCESS_2_MEMORY_WRITE_BIT) != 0) {
    result |= Access::memory_write;
  }

  return result;
}

} // namespace strobe::gpu
