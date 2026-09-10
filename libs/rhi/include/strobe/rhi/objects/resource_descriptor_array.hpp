#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
class ResourceDescriptorArray : public Object<ResourceDescriptorArray> {
  friend class Object<ResourceDescriptorArray>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
