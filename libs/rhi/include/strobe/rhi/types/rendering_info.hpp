#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/attachment.hpp"
#include "strobe/rhi/types/rect.hpp"
#include <cstdint>

namespace strobe::rhi {

struct RenderingInfo {
  span<const Attachment> colorAttachments = {};
  std::optional<Attachment> depthAttachment = std::nullopt;
  std::optional<Attachment> stencilAttachment = std::nullopt;

  Rect renderArea = {};
  uint32_t layerCount = 1;
  uint32_t viewMask = 0;
};

} // namespace strobe::rhi
