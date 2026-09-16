#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_array_impl.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap_impl.hpp"
#include "strobe/rhi/heap/sampler_descriptor_wizard.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/objects/sampler_descriptor_array.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include "strobe/rhi/types/sampler_descriptor_info.hpp"
#include <limits>
#include <memory>
#include <type_traits>
namespace strobe::rhi {

class SamplerDescriptorArrayWizard {
public:
  using descriptor = SamplerDescriptorArray;

  uint64_t size() const noexcept {
    auto *impl = object_handle_ptr<SamplerDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.descriptor_stride();
    return stride * m_infos.size();
  }

  uint64_t alignment() const noexcept {
    auto *impl = object_handle_ptr<SamplerDescriptorHeapImpl>(m_heap);
    return impl->layout.descriptor_stride();
  }

  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferRange>
  descriptor complete(void *dst, Fn &&fn) noexcept {

    auto *impl = object_handle_ptr<SamplerDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.descriptor_stride();
    const uint64_t offset = stride * static_cast<uint64_t>(m_index);
    Buffer heap_buf = m_heap.buffer();
    auto *heapbuf_impl = object_handle_ptr<BufferImpl>(heap_buf);
    heapbuf_impl->commit();

    SamplerDescriptor *descriptors =
        AllocatorTraits<strobe::rhi::allocator_ref>::allocate<
            SamplerDescriptor>(m_alloc, m_infos.size());

    std::byte *dst_bytes = static_cast<std::byte *>(dst);
    for (uint32_t i = 0; i < m_infos.size(); ++i) {
      SamplerDescriptorWizard wizard{m_heap, m_index + i, m_infos[i]};
      std::construct_at(descriptors + i,
                        std::move(wizard.complete(
                            dst_bytes + i * stride,
                            [](BufferRange) -> Timepoint { return {}; })));
    }
    Timepoint ready = fn(BufferRange{
        .buffer = heap_buf,
        .offset = offset,
        .size = m_infos.size() * stride,
    });
    for (uint32_t i = 0; i < m_infos.size(); ++i) {
      auto *desc = object_handle_ptr<SamplerDescriptorImpl>(descriptors[i]);
      desc->ready = ready;
    }

    return detail::make_object<SamplerDescriptorArray>(
        make_void_handle<SamplerDescriptorArrayImpl>(
            &impl->samplerDescArrayAlloc, std::move(m_heap),
            std::exchange(m_index, std::numeric_limits<uint32_t>::max()),
            m_infos.size(), std::move(ready), descriptors));
  }

  ~SamplerDescriptorArrayWizard() noexcept;

  explicit operator bool() const noexcept {
    return m_index != std::numeric_limits<uint32_t>::max();
  }

private:
  friend class SamplerDescriptorHeap;

  explicit SamplerDescriptorArrayWizard(
      SamplerDescriptorHeap heap, uint32_t index,
      span<const SamplerDescriptorInfo> infos,
      strobe::rhi::allocator_ref alloc) noexcept
      : m_heap(std::move(heap)), m_index(index), m_infos(infos),
        m_alloc(alloc) {}

  SamplerDescriptorHeap m_heap;
  uint32_t m_index;
  span<const SamplerDescriptorInfo> m_infos;
  [[no_unique_address]] strobe::rhi::allocator_ref m_alloc;
};

} // namespace strobe::rhi
