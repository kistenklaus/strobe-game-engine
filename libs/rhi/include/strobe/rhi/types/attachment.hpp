#pragma once

#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/types/attachment_load_op.hpp"
#include "strobe/rhi/types/attachment_store_op.hpp"
#include "strobe/rhi/types/clear_value.hpp"
#include "strobe/rhi/types/resolve_mode.hpp"

namespace strobe::rhi {

struct Attachment {
  ImageView view;
  AttachmentLoadOp loadOp = AttachmentLoadOp::load;
  AttachmentStoreOp storeOp = AttachmentStoreOp::store;
  ClearValue clearValue{};
  ResolveMode resolveMode = ResolveMode::none;
  ImageView resolveView = {};
};

} // namespace strobe::rhi
