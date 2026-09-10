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
  friend class Object<ResourceDescriptor>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
