#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/buffer_descriptor_array.hpp"
#include "strobe/rhi/heap/buffer_descriptor_array_impl.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include "strobe/rhi/utils/descriptor_type_utils.hpp"
#include <limits>
#include <type_traits>
#include <utility>

namespace strobe::rhi {

class BufferDescriptorArrayWizard {
public:
  using descriptor = BufferDescriptorArray;
  uint64_t size() const noexcept {
    const auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.buffer_stride();
    return stride * m_infos.size();
  }
  uint64_t alignment() const noexcept {
    const auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    return impl->layout.buffer_stride();
  }

  template <typename Fn>
    requires std::is_invocable_r_v<Timepoint, Fn, BufferRange>
  descriptor complete(void *dst, Fn &&fn) noexcept {
    auto *impl = object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap);
    const uint64_t stride = impl->layout.buffer_stride();
    const uint64_t offset = stride * static_cast<uint64_t>(m_index);
    vulkan::Context *ctx = m_heap.ctx();
    Buffer buffer = m_heap.buffer();
    auto *buffer_impl = object_handle_ptr<BufferImpl>(buffer);
    buffer_impl->commit();

    Vector<VkDeviceAddressRangeKHR, strobe::rhi::allocator_ref> addresses{
        m_infos.size(), m_alloc};
    Vector<VkResourceDescriptorInfoEXT, strobe::rhi::allocator_ref> resources{
        m_infos.size(), m_alloc};

    Vector<VkHostAddressRangeEXT, strobe::rhi::allocator_ref> descriptors{
        m_infos.size(), m_alloc};

    Buffer *buffers =
        AllocatorTraits<strobe::rhi::allocator_ref>::allocate<Buffer>(
            m_alloc, m_infos.size());

    for (uint32_t i = 0; i < m_infos.size(); ++i) {
      auto* resource_buf_impl = object_handle_ptr<BufferImpl>(m_infos[i].buffer);
      resource_buf_impl->commit();
      addresses[i] = VkDeviceAddressRangeKHR{
          .address = resource_buf_impl->address + offset + i * stride,
          .size = stride,
      };
      resources[i] = VkResourceDescriptorInfoEXT{
          .sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
          .pNext = nullptr,
          .type = to_vk_descriptor_type(m_infos[i].type),
          .data =
              VkResourceDescriptorDataEXT{
                  .pAddressRange = &addresses[i],
              },
      };
      descriptors[i] = VkHostAddressRangeEXT{
          .address = static_cast<std::byte *>(dst) + i * stride,
          .size = stride,
      };
    }

    {
      const VkResult result = vulkan::vk_write_resource_descriptors(
          ctx->pnf(), ctx->device(), m_infos.size(), resources.data(),
          descriptors.data());
      if (result != VK_SUCCESS) {
        vulkan_error(result, "Failed to write buffer resource descriptor.");
      }
    }

    Timepoint ready = fn(BufferRange{
        .buffer = buffer,
        .offset = offset,
        .size = m_infos.size() * stride,
    });

    return BufferDescriptorArray{make_void_handle<BufferDescriptorArrayImpl>(
        &impl->bufferDescArrayAlloc, std::move(m_heap),
        std::exchange(m_index, std::numeric_limits<uint32_t>::max()), m_infos.size(), std::move(ready), buffers,
        m_alloc)};
  }

  ~BufferDescriptorArrayWizard() noexcept {
    if (m_index != std::numeric_limits<uint32_t>::max()) {
      m_heap.release_buffer_descriptor_index_range(m_index, m_infos.size());
    }
  }

  explicit operator bool() const noexcept {
    return m_index != std::numeric_limits<uint32_t>::max();
  }

private:
  friend class ResourceDescriptorHeap;
  explicit BufferDescriptorArrayWizard(
      ResourceDescriptorHeap heap, uint32_t index,
      span<const BufferDescriptorInfo> infos,
      strobe::rhi::allocator_ref alloc) noexcept
      : m_heap(std::move(heap)), m_index(index), m_infos(infos),
        m_alloc(alloc) {}
  ResourceDescriptorHeap m_heap;
  uint32_t m_index;
  span<const BufferDescriptorInfo> m_infos;
  [[no_unique_address]] strobe::rhi::allocator_ref m_alloc;
};

} // namespace strobe::rhi
