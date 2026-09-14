#pragma once

#include "strobe/rhi/objects/object.hpp"

    namespace strobe::rhi {

  /**
   * \ingroup rhi
   * \brief Compute shader object.
   *
   * Defined in header <strobe/rhi/rhi.hpp>
   * \code{.cpp}
   * class ComputeShader : public Object<ComputeShader>;
   * \endcode
   *
   * Represents a device-owned compute shader object.
   */
  class ComputeShader : public Object<ComputeShader> {
    friend class Object<ComputeShader>;
    static void pin(void *) noexcept;
    static void unpin(void *) noexcept;

  public:
    using Object::Object;

    /**
     * \brief Sets debug name.
     * \code{.cpp}
     * void set_name(const char* name) noexcept;
     * \endcode
     *
     * Assigns a human-readable name to the shader object.
     *
     * \param name Null-terminated debug name.
     *
     * \attention 1. \p name must point to a valid null-terminated string.
     * \attention 2. validation layers must be enabled.
     */
    void set_name(const char *name) noexcept;
  };

} // namespace strobe::rhi
