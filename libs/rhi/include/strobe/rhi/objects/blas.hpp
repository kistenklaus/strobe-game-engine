#pragma once

#include "strobe/rhi/objects/object.hpp"
#include <cstdint>
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

public:
  using Object::Object;

  /**
   * \brief address of the blas
   * \code{cpp}
   * uint64_t address() const noexcept;
   * \endcode
   *
   * Returns the address of the Blas, or 0 if the object is null.
   */
  uint64_t address() const noexcept;
};

} // namespace strobe::rhi
