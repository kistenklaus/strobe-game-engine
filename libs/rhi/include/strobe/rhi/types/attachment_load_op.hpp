#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Attachment load operation.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet attachment_load_op.hpp AttachmentLoadOp
 *
 * Specifies how attachment contents are treated when rendering begins.
 */
// [AttachmentLoadOp]
enum class AttachmentLoadOp : uint8_t {
  load,
  clear,
  dont_care,
  none,
};
// [AttachmentLoadOp]

} // namespace strobe::rhi
