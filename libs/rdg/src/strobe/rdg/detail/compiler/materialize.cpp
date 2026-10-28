#include "compiler.hpp"
#include "materializer.hpp"
#include "strobe/core/containers/small_vector.hpp"
#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/rdg/types/attachment_immediates.hpp"
#include "strobe/rdg/types/buffer_info_immediates.hpp"
#include "util.hpp"

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/queue.hpp"

#include <memory>

namespace strobe::rdg::detail {

void materialize_operations(flow::detail::compiler::NormalizedIR &ir,
                            flow::CompileContext &context) {
  const auto &globals = compiler_state(context);
  IntrinsicMaterializer pass(ir, context, "strobe.rdg", 1);

  pass.define("create_cmdpool")
      .metadata("create-cmdpool")
      .arg_nonnull<rhi::CommandPool>(flow::AccessMode::write)
      .global<rhi::Device>(globals.device, flow::AccessMode::read)
      .body([](rhi::CommandPool *pool, const rhi::Device &device) noexcept {
        ZoneScopedN("create-cmdpool");
        std::construct_at(pool, device.create_cmdpool());
      });

  pass.define("create_buffer")
      .metadata("create-buffer")
      .immediate<BufferInfoImmediates>()
      .arg_nonnull<rhi::Buffer>(flow::AccessMode::write)
      .arg_nonnull<uint64_t>(flow::AccessMode::read)
      .global<rhi::Device>(globals.device, flow::AccessMode::read)
      .body([](const BufferInfoImmediates &imm, rhi::Buffer *buffer,
               const uint64_t &size, const rhi::Device &device,
               flow::Lifetime lifetime) noexcept {
        ZoneScopedN("create-buffer");
        std::construct_at(buffer, device.create_buffer(
                                      {
                                          .size = size,
                                          .bufferUsage = imm.bufferUsage,
                                          .memoryUsage = imm.memoryUsage,
                                      },
                                      {
                                          .begin = lifetime.begin,
                                          .end = lifetime.end,
                                      }));
      });

  pass.define("submit")
      .metadata("submit command buffer", flow::OpSemantics::side_effect)
      .arg_nonnull<rhi::Timepoint>(flow::AccessMode::write)
      .arg_nonnull<rhi::CommandBuffer>(flow::AccessMode::consume)
      .serialize<rhi::CommandPool>()
      .global<rhi::Queue>(globals.queue, flow::AccessMode::read)
      .serialize_global<rhi::Queue>(globals.queue)
      .body([](rhi::Timepoint *timepoint, rhi::CommandBuffer &cmd,
               const rhi::Queue &queue) noexcept {
        ZoneScopedN("rdg-submit");
        std::construct_at(
            timepoint, queue.submit(span<const rhi::CommandBuffer>(&cmd, 1)));
      });

  pass.define("timepoint_wait")
      .metadata("wait on timepoint completion")
      .immediate<uint64_t>()
      .arg_nonnull<rhi::Timepoint>(flow::AccessMode::read)
      .body([](const uint64_t &timeout, const rhi::Timepoint &timepoint) {
        ZoneScopedN("timepoint-wait");
        timepoint.wait(timeout);
      });

  pass.define("cmd_begin_rendering")
      .metadata("cmd_begin_rendering", flow::OpSemantics::side_effect)
      .immediate_span<AttachmentImmediates>()     // color attachment
      .immediate_optional<AttachmentImmediates>() // depth attachmnet
      .immediate_optional<AttachmentImmediates>() // stencil attachment
      .immediate<uint32_t>()                      // layer Count
      .immediate<uint32_t>()                      // view mask
      .arg_nonnull<rhi::CommandBuffer>(flow::AccessMode::read_write)
      .arg_span<rhi::ImageView>(flow::AccessMode::read)  // color views
      .arg_span<rhi::ImageView>(flow::AccessMode::read)  // color resolve views
      .arg_span<rhi::ClearValue>(flow::AccessMode::read) // color clear values
      .arg<rhi::ImageView>(flow::AccessMode::read)       // depth view
      .arg<rhi::ImageView>(flow::AccessMode::read)       // depth resolve view
      .arg<rhi::ClearValue>(flow::AccessMode::read)      // depth clear value
      .arg<rhi::ImageView>(flow::AccessMode::read)       // stencil view
      .arg<rhi::ImageView>(flow::AccessMode::read)       // stencil resolve view
      .arg<rhi::ClearValue>(flow::AccessMode::read)      // stencil clear value
      .arg<rhi::Rect>(flow::AccessMode::read)            // render-area
      .serialize<rhi::CommandPool>()
      .body([](span<const AttachmentImmediates> colorAttachments,          //
               const AttachmentImmediates *depthAttachment,                //
               const AttachmentImmediates *stencilAttachment,              //
               const uint32_t &layout_count,                               //
               const uint32_t &view_mask,                                  //
               rhi::CommandBuffer &cmd,                                    //
               flow::ArgumentSpan<const rhi::ImageView> colorViews,        //
               flow::ArgumentSpan<const rhi::ImageView> colorResolveViews, //
               flow::ArgumentSpan<const rhi::ClearValue> colorClearValues, //
               const rhi::ImageView *depthView,                            //
               const rhi::ImageView *depthResolveView,                     //
               const rhi::ClearValue *depthClearValue,                     //
               const rhi::ImageView *stencilView,                          //
               const rhi::ImageView *stencilResolveView,                   //
               const rhi::ClearValue *stencilClearValue,                   //
               const rhi::Rect *renderArea) noexcept {                     //
        ZoneScopedN("cmd-begin-rendering");
        rhi::RenderingInfo rhiInfo{};
        SmallVector<rhi::Attachment> catts{colorAttachments.size()};
        for (uint32_t i = 0; i < catts.size(); ++i) {
          const auto &imm = colorAttachments[i];
          auto &att = catts[i];
          assert(colorViews[i] != nullptr);
          att.view = *colorViews[i];
          att.layout = imm.layout;
          att.loadOp = imm.loadOp;
          att.storeOp = imm.storeOp;
          if (colorClearValues[i]) {
            att.clearValue = *colorClearValues[i];
          }
          att.resolveMode = imm.resolveMode;
          if (colorResolveViews[i]) {
            att.resolveView = *colorResolveViews[i];
          }
        }
        rhiInfo.colorAttachments = catts;
        if (depthAttachment) {
          const auto &imm = *depthAttachment;
          auto &att = rhiInfo.depthAttachment.emplace();
          assert(depthView);
          att.view = *depthView;
          att.layout = imm.layout;
          att.loadOp = imm.loadOp;
          att.storeOp = imm.storeOp;
          if (depthClearValue) {
            att.clearValue = *depthClearValue;
          }
          att.resolveMode = imm.resolveMode;
          if (depthResolveView) {
            att.resolveView = *depthResolveView;
          }
        }

        if (stencilAttachment) {
          const auto &imm = *stencilAttachment;
          auto &att = rhiInfo.stencilAttachment.emplace();
          assert(stencilView);
          att.view = *stencilView;
          att.layout = imm.layout;
          att.loadOp = imm.loadOp;
          att.storeOp = imm.storeOp;
          if (stencilClearValue) {
            att.clearValue = *stencilClearValue;
          }
          att.resolveMode = imm.resolveMode;
          if (stencilResolveView) {
            att.resolveView = *stencilResolveView;
          }
        }

        if (renderArea) {
          rhiInfo.renderArea = *renderArea;
        }
        rhiInfo.layerCount = layout_count;
        rhiInfo.viewMask = view_mask;
        cmd.begin_rendering(rhiInfo);
      });

  pass.define("cmd_end_rendering")
      .metadata("cmd_end_rendering", flow::OpSemantics::side_effect)
      .arg_nonnull<rhi::CommandBuffer>(flow::AccessMode::read_write)
      .serialize<rhi::CommandPool>()
      .body([](rhi::CommandBuffer &cmd) noexcept {
        ZoneScopedN("cmd-end-rendering");
        cmd.end_rendering();
      });

  pass.run();
}

} // namespace strobe::rdg::detail
