#pragma once

#include "strobe/rhi/objects/object.hpp"
#include <cassert>
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief A device-owned buffer.
 *
 * Buffers are created through Device and may be retained by command buffers.
 */
class Buffer : Object<Buffer> {
  friend class Device;
  friend class CommandBuffer;
  friend class MemoryPool;
  friend struct CommandBufferImpl;
  friend struct BlasImpl; // TODO: remove me

public:
  Buffer() noexcept : Object(nullptr) {}
  Buffer(const Buffer &) noexcept;
  Buffer(Buffer &&) noexcept;
  Buffer &operator=(const Buffer &) noexcept;
  Buffer &operator=(Buffer &&) noexcept;
  ~Buffer() noexcept;

  /**
   * \brief returns if the buffer is valid.
   */
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  /**
   * \brief compares two Buffers
   */
  friend bool operator==(const Buffer &lhs, const Buffer &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }

  /**
   * \brief compares two Buffers
   */
  friend bool operator!=(const Buffer &lhs, const Buffer &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  /**
   * \brief Returns the buffer size in bytes.
   */
  uint64_t size() const noexcept;

  /**
   * \brief Returns a mapped ptr of the buffer
   */
  void *ptr() const;

  /**
   * \brief set debug utils name
   */
  void set_name(const char *name) const noexcept;

  /**
   * \brief don't use.
   */
  explicit Buffer(void *handle) noexcept : Object(handle) { assert(handle); }
};
} // namespace strobe::rhi
