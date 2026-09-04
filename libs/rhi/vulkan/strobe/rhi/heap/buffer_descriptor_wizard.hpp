#pragma once

#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/buffer_descriptor_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/buffer_descriptor.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include "strobe/rhi/utils/descriptor_type_utils.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <limits>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

class BufferDescriptorWizard {
public:
  using descriptor = BufferDescriptor;

  uint64_t size() const noexcept {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    return impl->layout.buffer_stride();
  }
  uint64_t alignment() const noexcept {
    return size(); // may be a tigher but this still definitely be fine.
  }

  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferRange>
  descriptor complete(void *dst, Fn &&fn) noexcept {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.buffer_stride();
    const uint64_t offset = stride * static_cast<uint64_t>(m_index);
    vulkan::Context *ctx = m_heap.ctx();
    Buffer heap_buf = m_heap.buffer();
    auto *heapbuf_impl = object_handle_ptr<BufferImpl>(heap_buf);
    heapbuf_impl->commit();

    auto *resource_buf_impl = object_handle_ptr<BufferImpl>(m_info.buffer);
    resource_buf_impl->commit();

    assert(m_info.offset < resource_buf_impl->size);

    const uint64_t resourceSize = resource_buf_impl->size - m_info.offset;

    VkDeviceAddressRangeKHR address{
        .address = resource_buf_impl->address,
        .size = resourceSize,
    };
    VkResourceDescriptorInfoEXT resource{
        .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
        .pNext = nullptr,
        .type = to_vk_descriptor_type(m_info.type),
        .data =
            VkResourceDescriptorDataEXT{
                .pAddressRange = &address,
            },
    };
    VkHostAddressRangeEXT descriptor{
        .address = dst,
        .size = stride,
    };

    {
      const VkResult result = vulkan::vk_write_resource_descriptors(
          ctx->pnf(), ctx->device(), 1, &resource, &descriptor);
      if (result != VK_SUCCESS) {
        vulkan_error(result, "Failed to write buffer resource descriptor.");
      }
    }

    Timepoint ready = fn(BufferRange{
        .buffer = heap_buf,
        .offset = offset,
        .size = stride,
    });
    return BufferDescriptor{make_void_handle<BufferDescriptorImpl>(
        &impl->bufferDescAlloc, std::move(m_heap),
        std::exchange(m_index, std::numeric_limits<uint32_t>::max()),
        std::move(ready), m_info.buffer)};
  }
  ~BufferDescriptorWizard() noexcept {
    if (m_index != std::numeric_limits<uint32_t>::max()) {
      m_heap.release_buffer_descriptor_index_range(m_index, 1);
    }
  }

  explicit operator bool() const noexcept {
    return m_index != std::numeric_limits<uint32_t>::max();
  }

private:
  friend class ResourceDescriptorHeap;
  explicit BufferDescriptorWizard(ResourceDescriptorHeap heap, uint32_t index,
                                  const BufferDescriptorInfo &info) noexcept
      : m_heap(std::move(heap)), m_index(index), m_info(info) {}
  ResourceDescriptorHeap m_heap;
  uint32_t m_index;
  const BufferDescriptorInfo &m_info;
};

} // namespace strobe::rhi
