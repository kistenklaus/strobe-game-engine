#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Top-level acceleration structure.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{.cpp}
 * class Tlas : public Object<Tlas>;
 * \endcode
 *
 * Represents a device-owned top-level acceleration structure containing
 * instances of bottom-level acceleration structures.
 */
class Tlas : public Object<Tlas> {
  friend class Object<Tlas>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
