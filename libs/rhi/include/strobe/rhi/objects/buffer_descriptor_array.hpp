#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {


/**
 * \ingroup rhi
 * \brief todo
 */
class ResourceDescriptorArray : public Object<ResourceDescriptorArray> {
public:
  explicit ResourceDescriptorArray(void *handle) noexcept : Object(handle) {}
  ResourceDescriptorArray() noexcept : Object(nullptr) {}
  ResourceDescriptorArray(const ResourceDescriptorArray &) noexcept;
  ResourceDescriptorArray(ResourceDescriptorArray &&) noexcept;
  ResourceDescriptorArray &operator=(const ResourceDescriptorArray &) noexcept;
  ResourceDescriptorArray &operator=(ResourceDescriptorArray &&) noexcept;
  ~ResourceDescriptorArray() noexcept;
};

} // namespace strobe::rhi
