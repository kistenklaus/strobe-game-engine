#pragma once

#include "strobe/rhi/objects/object.hpp"
#include <cassert>
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief A device-owned buffer.
 */
class Buffer : public Object<Buffer> {
  friend Object<Buffer>;
  static void pin(void *handle) noexcept;
  static void unpin(void *handle) noexcept;

public:
  using Object::Object;
  /**
   * \brief pointer to memory mapped memory
   */
  void *ptr() const;

  /**
   * \brief size of the buffer in bytes
   */
  uint64_t size() const noexcept;

  /**
   * \brief set debug utils name
   */
  void set_name(const char *name) const noexcept;
};
} // namespace strobe::rhi
