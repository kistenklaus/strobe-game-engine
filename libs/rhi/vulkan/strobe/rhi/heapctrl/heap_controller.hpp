#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/buffer_descriptor.hpp"
#include "strobe/rhi/objects/buffer_descriptor_array.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/resource_descriptor_info.hpp"

namespace strobe::rhi {

class HeapController : Object<HeapController> {
public:
  explicit HeapController(void *handle) noexcept : Object(handle) {}
  HeapController() noexcept : Object(nullptr) {}
  HeapController(const HeapController &) noexcept;
  HeapController(HeapController &&) noexcept;
  HeapController &operator=(const HeapController &) noexcept;
  HeapController &operator=(HeapController &&) noexcept;
  ~HeapController() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const HeapController &lhs,
                         const HeapController &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const HeapController &lhs,
                         const HeapController &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  ResourceDescriptor
  create_resource_descriptor(const ResourceDescriptorInfo &info) noexcept;

  ResourceDescriptorArray create_resource_descriptor_array(
      span<const ResourceDescriptorInfo> infos) noexcept;
};

} // namespace strobe::rhi
