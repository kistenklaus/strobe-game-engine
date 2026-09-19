#pragma once

#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief geometry shader object.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{cpp}
 * class GeometryShader : public Object<GeometryShader>;
 * \endcode
 *
 * Represents a device-owned geometry shader object.
 */
class GeometryShader : public Object<GeometryShader> {
  friend class Object<GeometryShader>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
