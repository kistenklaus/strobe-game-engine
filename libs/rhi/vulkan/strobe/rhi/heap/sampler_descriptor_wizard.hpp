#pragma once

#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap_impl.hpp"
#include "strobe/rhi/heap/sampler_descriptor_impl.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include "strobe/rhi/types/sampler_descriptor_info.hpp"
#include "strobe/rhi/utils/border_color_utils.hpp"
#include "strobe/rhi/utils/compare_op_utils.hpp"
#include "strobe/rhi/utils/filter_utils.hpp"
#include "strobe/rhi/utils/sampler_address_mode_utils.hpp"
#include "strobe/rhi/utils/sampler_mipmap_mode_utils.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vulkan/vulkan_core.h>
namespace strobe::rhi {

class SamplerDescriptorWizard {
public:
  using descriptor = SamplerDescriptor;

  uint64_t size() const noexcept {
    auto *impl = object_handle_ptr<SamplerDescriptorHeapImpl>(m_heap);
    return impl->layout.descriptor_stride();
  }

  uint64_t alignment() const noexcept {
    return size(); // may be a tigher but this still definitely be fine.
  }

  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferRange>
  descriptor complete(void *dst, Fn &&fn) noexcept {
    auto *impl = object_handle_ptr<SamplerDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.descriptor_stride();
    const uint64_t offset = stride * static_cast<uint64_t>(m_index);
    vulkan::Context *ctx = m_heap.ctx();
    Buffer heap_buf = m_heap.buffer();
    auto *heapbuf_impl = object_handle_ptr<BufferImpl>(heap_buf);
    heapbuf_impl->commit();

    VkSamplerCreateInfo samplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = to_vk_filter(m_info.magFilter),
        .minFilter = to_vk_filter(m_info.minFilter),
        .mipmapMode = to_vk_sampler_mipmap_mode(m_info.mipmapMode),
        .addressModeU = to_vk_sampler_address_mode(m_info.addressModeU),
        .addressModeV = to_vk_sampler_address_mode(m_info.addressModeV),
        .addressModeW = to_vk_sampler_address_mode(m_info.addressModeW),
        .mipLodBias = m_info.mipLodBias,
        .anisotropyEnable = m_info.anisotropyEnable,
        .maxAnisotropy = m_info.maxAnisotropy,
        .compareEnable = m_info.compareEnable,
        .compareOp = to_vk_compare_op(m_info.compareOp),
        .minLod = m_info.minLod,
        .maxLod = std::min(VK_LOD_CLAMP_NONE, m_info.maxLod),
        .borderColor = to_vk_border_color(m_info.borderColor),
        .unnormalizedCoordinates = m_info.unnormalizedCoordinates,
    };

    VkHostAddressRangeEXT descriptor{
        .address = dst,
        .size = stride,
    };

    {
#ifdef STROBE_RHI_TRACE_VK
      ZoneScopedN("vkWriteSamplerDescriptors");
#endif
      const VkResult result = vulkan::vk_write_sampler_descriptors(
          ctx->pnf(), ctx->device(), 1, &samplerInfo, &descriptor);
      if (result != VK_SUCCESS) {
        vulkan_error(result, "Failed to write sampler descriptor.");
      }
    }

    Timepoint ready = fn(BufferRange{
        .buffer = heap_buf,
        .offset = offset,
        .size = stride,
    });

    return detail::make_object<SamplerDescriptor>(
        make_void_handle<SamplerDescriptorImpl>(
            &impl->samplerDescAlloc, std::move(m_heap),
            std::exchange(m_index, std::numeric_limits<uint32_t>::max()),
            std::move(ready)));
  }

  explicit operator bool() const noexcept {
    return m_index != std::numeric_limits<uint32_t>::max();
  }

  ~SamplerDescriptorWizard() noexcept;

private:
  friend class SamplerDescriptorHeap;
  friend class SamplerDescriptorArrayWizard;

  explicit SamplerDescriptorWizard(SamplerDescriptorHeap heap, uint32_t index,
                                   const SamplerDescriptorInfo &info) noexcept
      : m_heap(std::move(heap)), m_index(index), m_info(info) {}

  SamplerDescriptorHeap m_heap;
  uint32_t m_index;
  const SamplerDescriptorInfo &m_info;
};

} // namespace strobe::rhi
