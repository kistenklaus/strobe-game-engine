#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief A resource descriptor
 *
 * Resources are buffers and images ...
 */
class ResourceDescriptor : public Object<ResourceDescriptor> {
public:
  explicit ResourceDescriptor(void *handle) noexcept : Object(handle) {}
  ResourceDescriptor() noexcept : Object(nullptr) {}
  ResourceDescriptor(const ResourceDescriptor &) noexcept;
  ResourceDescriptor(ResourceDescriptor &&) noexcept;
  ResourceDescriptor &operator=(const ResourceDescriptor &) noexcept;
  ResourceDescriptor &operator=(ResourceDescriptor &&) noexcept;
  ~ResourceDescriptor() noexcept;
};

} // namespace strobe::rhi
