#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
enum class AttachmentLoadOp : uint8_t {
  load,
  clear,
  dont_care,
  none,
};

} // namespace strobe::rhi
