#pragma once

#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/descriptor_pool.hpp"
#include "strobe/gpu/vulkan/descriptor_set_layout.hpp"
#include "strobe/gpu/vulkan/image_view.hpp"
#include <variant>
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct DescriptorSet {
  VkDescriptorSet handle = VK_NULL_HANDLE;

  explicit operator bool() const noexcept { return handle != VK_NULL_HANDLE; }
};

struct DescriptorSetInfo {
  DescriptorPool pool = {};
  span<const DescriptorSetLayout> layouts = {};
};

struct DescriptorBufferInfo {
  Buffer buffer;
  VkDeviceSize offset = 0;
  VkDeviceSize size = VK_WHOLE_SIZE;
};

struct DescriptorImageInfo {
  Sampler sampler = {};
  ImageView view = {};
  VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
};

struct WriteDescriptorInfo {
  DescriptorSet set = {};
  uint32_t binding = 0;
  uint32_t arrayElement = 0;
  VkDescriptorType type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  std::variant<span<const DescriptorBufferInfo>,
               span<const DescriptorImageInfo>>
      what;
};

struct CopyDescriptorInfo {
  DescriptorSet srcSet = {};
  uint32_t srcBinding = 0;
  uint32_t srcArrayElement = 0;
  DescriptorSet dstSet = {};
  uint32_t dstBinding = 0;
  uint32_t dstArrayElement = 0;

  uint32_t count = 1;
};

struct DescriptorUpdateInfo {
  span<const WriteDescriptorInfo> writes = {};
  span<const CopyDescriptorInfo> copies = {};
};

void alloc_descriptor_sets(Context *context, const DescriptorSetInfo &info,
                           span<DescriptorSet> out);

void free_descriptor_sets(Context *context, DescriptorPool pool,
                          span<const DescriptorSet> sets);

void update_descriptor_sets(Context *context,
                            const DescriptorUpdateInfo &info = {});

} // namespace strobe::gpu::vulkan
