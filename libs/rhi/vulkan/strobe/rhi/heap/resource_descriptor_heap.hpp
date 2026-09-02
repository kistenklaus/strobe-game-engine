#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/heap/descriptor_heap_bind_info.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/buffer_range.hpp"
#include <cstdint>

namespace strobe::rhi {

// fwd declarations.
class BufferDescriptorWizard;
class BufferDescriptorArrayWizard;

class ResourceDescriptorHeap : Object<ResourceDescriptorHeap> {
public:
  explicit ResourceDescriptorHeap(void *handle) noexcept : Object(handle) {}
  ResourceDescriptorHeap() noexcept : Object(nullptr) {}
  ResourceDescriptorHeap(const ResourceDescriptorHeap &) noexcept;
  ResourceDescriptorHeap(ResourceDescriptorHeap &&) noexcept;
  ResourceDescriptorHeap &operator=(const ResourceDescriptorHeap &) noexcept;
  ResourceDescriptorHeap &operator=(ResourceDescriptorHeap &&) noexcept;
  ~ResourceDescriptorHeap() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const ResourceDescriptorHeap &lhs,
                         const ResourceDescriptorHeap &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const ResourceDescriptorHeap &lhs,
                         const ResourceDescriptorHeap &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  BufferDescriptorWizard create_buffer_descriptor_wizard(BufferRange buffer) noexcept;
  BufferDescriptorArrayWizard
  create_buffer_descriptor_array_wizard(uint32_t size, span<const BufferRange> buffers) noexcept;

  void release_buffer_descriptor_index_range(uint32_t index,
                                             uint32_t count) const noexcept;
  void release_image_descriptor_index_range(uint32_t index,
                                            uint32_t count) const noexcept;

  Buffer buffer() const noexcept;

  DescriptorHeapBindInfo bindInfo() const noexcept;
};

} // namespace strobe::rhi
