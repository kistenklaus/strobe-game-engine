#pragma once

#include "strobe/rhi/types/format.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
struct VertexAttribute {
  uint32_t location;
  uint32_t binding;
  Format format;
  uint32_t offset;
};

} // namespace strobe::rhi
