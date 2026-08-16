#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/gpu/device/attachment.hpp"
#include "strobe/gpu/device/rect.hpp"
#include <cstdint>

namespace strobe::gpu {

struct RenderingInfo {
  span<const Attachment> colorAttachments = {};
  std::optional<Attachment> depthAttachment = std::nullopt;
  std::optional<Attachment> stencilAttachment = std::nullopt;

  Rect renderArea = {};
  uint32_t layerCount = 1;
  uint32_t viewMask = 0;
};

} // namespace strobe::gpu
