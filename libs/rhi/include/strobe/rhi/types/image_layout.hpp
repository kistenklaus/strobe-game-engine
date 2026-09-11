#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image memory layout.
 *
 * Specifies the memory layout and intended access mode of an image.
 */
enum class ImageLayout : uint8_t {
  undefined,    ///< Previous image contents are not preserved.
  general,      ///< General-purpose image layout.

  read_only,    ///< Read-only image access.
  attachment,   ///< Rendering attachment access.

  transfer_src, ///< Transfer source access.
  transfer_dst, ///< Transfer destination access.

  present,      ///< Presentation layout.
};

} // namespace strobe::rhi
