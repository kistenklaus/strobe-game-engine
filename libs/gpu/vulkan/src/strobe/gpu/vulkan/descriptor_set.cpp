#include "strobe/gpu/vulkan/descriptor_set.hpp"

#include <stdexcept>

namespace strobe::gpu::vulkan {

void alloc_descriptor_sets(Context *context, const DescriptorSetInfo &info,
                           span<DescriptorSet> out) {
  assert(context != nullptr);
  static constexpr size_t SCRATCH_SIZE =
      sizeof(VkDescriptorSetLayout) * 24 + sizeof(VkDescriptorSet) * 24;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};

  uint32_t count = info.layouts.size();
  Vector<VkDescriptorSetLayout, scratch_allocator_ref> layouts{count, &scratch};
  for (uint32_t i = 0; i < count; ++i) {
    assert(info.layouts[i]);
    layouts[i] = info.layouts[i].handle;
  }

  VkDescriptorSetAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = nullptr,
      .descriptorPool = info.pool.handle,
      .descriptorSetCount = count,
      .pSetLayouts = layouts.data(),
  };
  Vector<VkDescriptorSet, scratch_allocator_ref> sets{count, &scratch};

  VkResult result =
      vkAllocateDescriptorSets(context->device(), &allocInfo, sets.data());
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor sets");
  }

  for (uint32_t i = 0; i < count; ++i) {
    assert(sets[i] != VK_NULL_HANDLE);
    out[i] = DescriptorSet{.handle = sets[i]};
  }
}
void free_descriptor_sets(Context *context, DescriptorPool pool,
                          span<const DescriptorSet> sets) {
  static constexpr size_t SCRATCH_SIZE = sizeof(VkDescriptorSet) * 24;
  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};

  Vector<VkDescriptorSet, scratch_allocator_ref> free{sets.size(), &scratch};
  for (size_t i = 0; i < sets.size(); ++i) {
    assert(sets[i]);
    free[i] = sets[i].handle;
  }

  VkResult result =
      vkFreeDescriptorSets(context->device(), pool.handle,
                           static_cast<uint32_t>(free.size()), free.data());
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to free descriptor sets");
  }
}

void update_descriptor_sets(Context *context,
                            const DescriptorUpdateInfo &info) {
  assert(context != nullptr);
  static constexpr size_t SCRATCH_SIZE =
      sizeof(VkWriteDescriptorSet) * 24 + sizeof(VkCopyDescriptorSet) * 24 +
      std::max(sizeof(DescriptorBufferInfo), sizeof(DescriptorImageInfo)) * 24;

  using scratch_allocator =
      InplaceMonotonicResource<strobe::Mallocator, SCRATCH_SIZE>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;
  scratch_allocator scratch{};

  Vector<VkWriteDescriptorSet, scratch_allocator_ref> writeInfos{
      info.writes.size(), &scratch};
  for (size_t i = 0; i < info.writes.size(); ++i) {

    VkDescriptorImageInfo *pImageInfos = nullptr;
    VkDescriptorBufferInfo *pBufferInfos = nullptr;
    uint32_t count = 0;

    if (std::holds_alternative<span<const DescriptorBufferInfo>>(
            info.writes[i].what)) {
      auto bufferInfos =
          std::get<span<const DescriptorBufferInfo>>(info.writes[i].what);
      count = bufferInfos.size();
      pBufferInfos = static_cast<VkDescriptorBufferInfo *>(
          scratch.allocate(count * sizeof(VkDescriptorBufferInfo),
                           alignof(VkDescriptorBufferInfo)));
      for (uint32_t j = 0; j < count; ++j) {
        assert(bufferInfos[j].buffer);
        pBufferInfos[i] = VkDescriptorBufferInfo{
            .buffer = bufferInfos[j].buffer.handle,
            .offset = bufferInfos[j].offset,
            .range = bufferInfos[j].size,
        };
      }
    } else if (std::holds_alternative<span<const DescriptorImageInfo>>(
                   info.writes[i].what)) {
      auto imageInfos =
          std::get<span<const DescriptorImageInfo>>(info.writes[i].what);
      count = imageInfos.size();
      pImageInfos = static_cast<VkDescriptorImageInfo *>(
          scratch.allocate(count * sizeof(VkDescriptorImageInfo),
                           alignof(VkDescriptorImageInfo)));
      for (uint32_t j = 0; j < count; ++j) {
        assert(imageInfos[j].sampler);
        assert(imageInfos[j].view);
        pImageInfos[j] =
            VkDescriptorImageInfo{.sampler = imageInfos[j].sampler.handle,
                                  .imageView = imageInfos[j].view.handle,
                                  .imageLayout = imageInfos[j].layout};
      }
    }
    writeInfos[i] = VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = info.writes[i].set.handle,
        .dstBinding = info.writes[i].binding,
        .dstArrayElement = info.writes[i].arrayElement,
        .descriptorCount = count,
        .descriptorType = info.writes[i].type,
        .pImageInfo = pImageInfos,
        .pBufferInfo = pBufferInfos,
        .pTexelBufferView = nullptr,
    };
  }

  Vector<VkCopyDescriptorSet, scratch_allocator_ref> copyInfos{
      info.copies.size(), &scratch};
  for (size_t i = 0; i < info.copies.size(); ++i) {
    copyInfos[i] = VkCopyDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
        .pNext = nullptr,
        .srcSet = info.copies[i].srcSet.handle,
        .srcBinding = info.copies[i].srcBinding,
        .srcArrayElement = info.copies[i].srcArrayElement,
        .dstSet = info.copies[i].dstSet.handle,
        .dstBinding = info.copies[i].dstBinding,
        .dstArrayElement = info.copies[i].dstArrayElement,
        .descriptorCount = info.copies[i].count,
    };
  }

  vkUpdateDescriptorSets(
      context->device(), static_cast<uint32_t>(writeInfos.size()),
      writeInfos.data(), static_cast<uint32_t>(copyInfos.size()),
      copyInfos.data());
}

} // namespace strobe::gpu::vulkan
