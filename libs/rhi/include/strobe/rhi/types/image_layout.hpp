#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
enum class ImageLayout : uint8_t {
  undefined,
  general,

  read_only,
  attachment,

  transfer_src,
  transfer_dst,

  present,
};

}
