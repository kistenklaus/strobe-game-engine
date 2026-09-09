#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_array_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_wizard.hpp"
#include "strobe/rhi/objects/buffer_descriptor_array.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include <limits>
#include <type_traits>
#include <utility>

namespace strobe::rhi {

class ResourceDescriptorArrayWizard {
public:
  using descriptor = ResourceDescriptorArray;
  uint64_t size() const noexcept {
    const auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.descriptor_stride();
    return stride * m_infos.size();
  }
  uint64_t alignment() const noexcept {
    const auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    return impl->layout.descriptor_stride();
  }

  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferRange>
  descriptor complete(void *dst, Fn &&fn) noexcept {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.descriptor_stride();
    const uint64_t offset = stride * static_cast<uint64_t>(m_index);
    Buffer heap_buf = m_heap.buffer();
    auto *heapbuf_impl = object_handle_ptr<BufferImpl>(heap_buf);
    heapbuf_impl->commit();

    ResourceDescriptor *descriptors =
        AllocatorTraits<strobe::rhi::allocator_ref>::allocate<
            ResourceDescriptor>(m_alloc, m_infos.size());

    std::byte *dst_bytes = static_cast<std::byte *>(dst);
    for (uint32_t i = 0; i < m_infos.size(); ++i) {
      ResourceDescriptorWizard wizard{m_heap, m_index + i, m_infos[i]};
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
      auto *desc =
          object_handle_ptr<ResourceDescriptorArrayImpl>(descriptors[i]);
      desc->ready = ready;
    }

    return ResourceDescriptorArray{
        make_void_handle<ResourceDescriptorArrayImpl>(
            &impl->bufferDescArrayAlloc, std::move(m_heap),
            std::exchange(m_index, std::numeric_limits<uint32_t>::max()),
            m_infos.size(), std::move(ready), descriptors)};
  }

  ~ResourceDescriptorArrayWizard() noexcept;

  explicit operator bool() const noexcept {
    return m_index != std::numeric_limits<uint32_t>::max();
  }

private:
  friend class ResourceDescriptorHeap;
  explicit ResourceDescriptorArrayWizard(
      ResourceDescriptorHeap heap, uint32_t index,
      span<const ResourceDescriptorInfo> infos,
      strobe::rhi::allocator_ref alloc) noexcept
      : m_heap(std::move(heap)), m_index(index), m_infos(infos),
        m_alloc(alloc) {}
  ResourceDescriptorHeap m_heap;
  uint32_t m_index;
  span<const ResourceDescriptorInfo> m_infos;
  [[no_unique_address]] strobe::rhi::allocator_ref m_alloc;
};

} // namespace strobe::rhi
