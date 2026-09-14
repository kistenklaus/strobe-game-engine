#pragma once

#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/types/image_view_type.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image view creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} image_view_info.hpp ImageViewInfo
 *
 * Describes the type, format, and subresource range of an image view.
 */
// [ImageViewInfo]
struct ImageViewInfo {
  ImageViewType type = ImageViewType::none;
  Format format = Format::undefined;
  ImageSubresourceRange range = {};
};
// [ImageViewInfo]

} // namespace strobe::rhi
