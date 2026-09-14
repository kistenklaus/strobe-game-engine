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
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} rendering_info.hpp RenderingInfo
 *
 * Describes attachments and render area used for a dynamic rendering instance.
 */
// [RenderingInfo]
struct RenderingInfo {
  span<const Attachment> colorAttachments = {};
  std::optional<Attachment> depthAttachment = std::nullopt;
  std::optional<Attachment> stencilAttachment = std::nullopt;
  Rect renderArea = {};
  uint32_t layerCount = 1;
  uint32_t viewMask = 0;
};
// [RenderingInfo]

} // namespace strobe::rhi
