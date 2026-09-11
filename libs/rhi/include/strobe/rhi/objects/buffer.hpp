#pragma once

#include "strobe/rhi/objects/object.hpp"
#include <cassert>
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Linear array of data.
 *
 * Defined in header <strobe/rhi/rhi.hpp>
 *
 * \code{.cpp}
 * class Buffer : public Object<Buffer>;
 * \endcode
 *
 * Object handle to a linear array of device-owned data.
 *
 */
class Buffer : public Object<Buffer> {
  friend Object<Buffer>;
  static void pin(void *handle) noexcept;
  static void unpin(void *handle) noexcept;

public:
  using Object::Object;
  /**
   * \brief Returns pointer to memory mapped memory
   * \code{.cpp}
   * void* ptr() const;
   * \endcode
   *
   * \attention Buffer must have been created with a mapped MemoryUsage.
   */
  void *ptr() const;

  /**
   * \brief Returns size of the buffer in bytes.
   * \code{.cpp}
   * uint64_t size() const noexcept;
   * \endcode
   */
  uint64_t size() const noexcept;

  /**
   * \brief Set debug utils name
   * \code{.cpp}
   * void set_name(const char* name) const noexcept;
   * \endcode
   * \attention debug utils must have been enabled at device creation.
   */
  void set_name(const char *name) const noexcept;
};
} // namespace strobe::rhi
