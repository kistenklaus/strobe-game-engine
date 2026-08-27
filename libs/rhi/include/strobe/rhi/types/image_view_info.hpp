#pragma once

#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/types/image_view_type.hpp"

namespace strobe::rhi {

struct ImageViewInfo {
  ImageViewType type = ImageViewType::none;
  Format format = Format::undefined; // undefined => inherit from image
  ImageSubresourceRange range = {};
};

}
