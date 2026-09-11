#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Fragment shader object.
 * Defined in header <strobe/rhi/rhi.hpp>
 * 
 * \code{.cpp}
 * class FragmentShader : public Object<FragmentShader>;
 * \endcode
 *
 * Represents a device-owned fragment shader object.
 */
class FragmentShader : public Object<FragmentShader> {
  friend class Object<FragmentShader>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
};

} // namespace strobe::rhi
