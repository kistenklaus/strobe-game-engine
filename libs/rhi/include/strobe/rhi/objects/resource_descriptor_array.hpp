#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Resource descriptor array.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{.cpp}
 * class ResourceDescriptorArray : public Object<ResourceDescriptorArray>;
 * \endcode
 *
 * Represents a contiguous range of resource descriptors stored in the device
 * resource descriptor heap.
 */
class ResourceDescriptorArray : public Object<ResourceDescriptorArray> {
  friend class Object<ResourceDescriptorArray>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
