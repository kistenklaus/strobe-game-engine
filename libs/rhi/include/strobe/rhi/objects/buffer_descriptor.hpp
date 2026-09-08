#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

class ResourceDescriptor : Object<ResourceDescriptor> {
public:
  explicit ResourceDescriptor(void *handle) noexcept : Object(handle) {}
  ResourceDescriptor() noexcept : Object(nullptr) {}
  ResourceDescriptor(const ResourceDescriptor &) noexcept;
  ResourceDescriptor(ResourceDescriptor &&) noexcept;
  ResourceDescriptor &operator=(const ResourceDescriptor &) noexcept;
  ResourceDescriptor &operator=(ResourceDescriptor &&) noexcept;
  ~ResourceDescriptor() noexcept;

  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const ResourceDescriptor &lhs,
                         const ResourceDescriptor &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const ResourceDescriptor &lhs,
                         const ResourceDescriptor &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }
};

} // namespace strobe::rhi
