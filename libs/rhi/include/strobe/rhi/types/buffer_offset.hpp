#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
struct BufferOffset {
  Buffer buffer{};
  uint64_t offset = 0;
};

} // namespace strobe::rhi
