#pragma once

#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi {
/**
 * \ingroup rhi
 * \brief Bottom Level Acceleration Structure
 *
 * Defined in header <strobe/rhi/rhi.hpp>
 *
 * \code{.cpp}
 * class Blas : public Object<Blas>;
 * \endcode
 *
 * Object handle to a bottom level acceleration structure.
 *
 * Blas are created and owned by the Device.
 */
class Blas : public Object<Blas> {
  friend class Object<Blas>;
  static void pin(void *handle) noexcept;
  static void unpin(void *handle) noexcept;
  using Object::Object;
};

} // namespace strobe::rhi
