#pragma once

#include "strobe/rhi/types/attachment_load_op.hpp"
#include "strobe/rhi/types/attachment_store_op.hpp"
#include "strobe/rhi/types/clear_value.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/resolve_mode.hpp"
namespace strobe::rdg {

struct AttachmentImmediates {
  rhi::ImageLayout layout;
  rhi::AttachmentLoadOp loadOp;
  rhi::AttachmentStoreOp storeOp;
  rhi::ResolveMode resolveMode;
};

} // namespace strobe::rdg
