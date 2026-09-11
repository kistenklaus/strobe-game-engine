#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Attachment store operation.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{.cpp} attachment_store_op.hpp AttachmentStoreOp
 *
 * Specifies how attachment contents are treated when rendering ends.
 */
// [AttachmentStoreOp]
enum class AttachmentStoreOp : uint8_t {
  store,     ///< Store the attachment contents.
  dont_care, ///< Attachment contents may be discarded.
  none,      ///< No store operation is performed.
};
// [AttachmentStoreOp]

} // namespace strobe::rhi
