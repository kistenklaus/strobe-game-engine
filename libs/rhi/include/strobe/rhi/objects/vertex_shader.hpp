#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Vertex shader object.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{.cpp}
 * class VertexShader : public Object<VertexShader>;
 * \endcode
 *
 * Represents a device-owned vertex shader object.
 */
class VertexShader : public Object<VertexShader> {
  friend class Object<VertexShader>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;
};

} // namespace strobe::rhi
