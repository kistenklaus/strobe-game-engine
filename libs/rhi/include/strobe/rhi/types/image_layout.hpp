#pragma once

#include <cstdint>

namespace strobe::rhi {

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
