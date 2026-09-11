#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Resource descriptor handle.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{.cpp}
 * class ResourceDescriptor : public Object<ResourceDescriptor>;
 * \endcode
 *
 * Represents a descriptor of a buffer or image stored in the device resource
 * descriptor heap.
 */
class ResourceDescriptor : public Object<ResourceDescriptor> {
  friend class Object<ResourceDescriptor>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
