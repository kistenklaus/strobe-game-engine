#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/types/attachment.hpp"
#include "strobe/rhi/types/rect.hpp"
#include <cstdint>
#include <optional>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Dynamic rendering information.
 *
 * Describes attachments and render area used for a dynamic rendering instance.
 */
struct RenderingInfo {
  /** Color attachments used for rendering. */
  span<const Attachment> colorAttachments = {};

  /** Optional depth attachment. */
  std::optional<Attachment> depthAttachment = std::nullopt;

  /** Optional stencil attachment. */
  std::optional<Attachment> stencilAttachment = std::nullopt;

  /** Render area in framebuffer coordinates. */
  Rect renderArea = {};

  /** Number of rendered layers. */
  uint32_t layerCount = 1;

  /** Multiview view mask. */
  uint32_t viewMask = 0;
};

} // namespace strobe::rhi
