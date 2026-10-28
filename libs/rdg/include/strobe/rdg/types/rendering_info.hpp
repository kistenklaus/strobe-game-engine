#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rdg/types/attachment.hpp"
#include "strobe/rdg/types/rect.hpp"

namespace strobe::rdg {

struct RenderingInfo {
  span<const Attachment> colorAttachments{};
  std::optional<Attachment> depthAttachment{};
  std::optional<Attachment> stencilAttachment{};
  std::optional<Rect> renderArea{};
  uint32_t layerCount = 1;
  uint32_t viewMask = 0;
};

} // namespace strobe::rdg
