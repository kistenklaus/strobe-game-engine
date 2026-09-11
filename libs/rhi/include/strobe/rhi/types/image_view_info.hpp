#pragma once

#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/types/image_view_type.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Image view creation information.
 *
 * Describes the type, format, and subresource range of an image view.
 */
struct ImageViewInfo {
  /** Image view type. */
  ImageViewType type = ImageViewType::none;

  /** View format, or Format::undefined to inherit the image format. */
  Format format = Format::undefined;

  /** Image subresources visible through the view. */
  ImageSubresourceRange range = {};
};

} // namespace strobe::rhi
