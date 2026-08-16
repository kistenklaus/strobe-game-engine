#pragma once

#include "strobe/gpu/device/attachment_load_op.hpp"
#include "strobe/gpu/device/attachment_store_op.hpp"
#include "strobe/gpu/device/clear_value.hpp"
#include "strobe/gpu/device/image_view.hpp"
#include "strobe/gpu/device/resolve_mode.hpp"

namespace strobe::gpu {

struct Attachment {
  ImageView view;
  AttachmentLoadOp loadOp = AttachmentLoadOp::load;
  AttachmentStoreOp storeOp = AttachmentStoreOp::store;
  ClearValue clearValue{};
  ResolveMode resolveMode = ResolveMode::none;
  ImageView resolveView = {};
};

} // namespace strobe::gpu
