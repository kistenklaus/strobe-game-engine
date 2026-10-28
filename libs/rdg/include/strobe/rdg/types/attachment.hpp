#pragma once

#include "strobe/rdg/image.hpp"
#include "strobe/rdg/types/clear_value.hpp"
#include "strobe/rhi/types/attachment_load_op.hpp"
#include "strobe/rhi/types/attachment_store_op.hpp"
#include "strobe/rhi/types/clear_value.hpp"
#include "strobe/rhi/types/resolve_mode.hpp"

namespace strobe::rdg {

struct Attachment {
  ImageView view;
  rhi::ImageLayout layout = rhi::ImageLayout::attachment;
  rhi::AttachmentLoadOp loadOp = rhi::AttachmentLoadOp::load;
  rhi::AttachmentStoreOp storeOp = rhi::AttachmentStoreOp::store;
  std::optional<ClearValue> clearValue{};
  rhi::ResolveMode resolveMode = rhi::ResolveMode::none;
  std::optional<ImageView> resolveView{};
};

} // namespace strobe::rdg
