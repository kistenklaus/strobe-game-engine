#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

class ResourceDescriptorArray : Object<ResourceDescriptorArray> {
public:
  explicit ResourceDescriptorArray(void *handle) noexcept : Object(handle) {}
  ResourceDescriptorArray() noexcept : Object(nullptr) {}
  ResourceDescriptorArray(const ResourceDescriptorArray &) noexcept;
  ResourceDescriptorArray(ResourceDescriptorArray &&) noexcept;
  ResourceDescriptorArray &operator=(const ResourceDescriptorArray &) noexcept;
  ResourceDescriptorArray &operator=(ResourceDescriptorArray &&) noexcept;
  ~ResourceDescriptorArray() noexcept;

  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const ResourceDescriptorArray &lhs,
                         const ResourceDescriptorArray &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const ResourceDescriptorArray &lhs,
                         const ResourceDescriptorArray &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }
};

} // namespace strobe::rhi
