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
  /** \name (constructors)
   * @{
   */
  /**
   * \brief default constructor
   */
  Buffer() noexcept : Object(nullptr) {}
  /**
   * \brief copy-constructor
   */
  Buffer(const Buffer &) noexcept;
  /**
   * \brief move-constructor
   */
  Buffer(Buffer &&) noexcept;
  /**
   * \brief copy-assignment
   */
  Buffer &operator=(const Buffer &) noexcept;
  /**
   * \brief move-assignment
   */
  Buffer &operator=(Buffer &&) noexcept;
  /**
   * \brief destructor
   */
  ~Buffer() noexcept;
  /**
   * @}
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
   * \name Interface
   * @{
   */

  /**
   * \brief Returns a mapped ptr of the buffer
   */
  void *ptr() const;

  /**
   * \brief Returns the buffer size in bytes.
   */
  uint64_t size() const noexcept;

  /**
   * @}
   */

  /** 
   * \name Debug Utils
   * @{
   */

  /**
   * \brief set debug utils name
   */
  void set_name(const char *name) const noexcept;

  /**
   * @}
   */


  explicit Buffer(void *handle) noexcept : Object(handle) { assert(handle); }
};
} // namespace strobe::rhi
