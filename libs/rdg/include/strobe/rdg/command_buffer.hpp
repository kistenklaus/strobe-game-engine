#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/rc.hpp"
#include "strobe/flow/access.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/resource.hpp"
#include "strobe/rdg/buffer.hpp"
#include "strobe/rdg/swapchain.hpp"
#include "strobe/rdg/timepoint.hpp"
#include "strobe/rdg/types/attachment_immediates.hpp"
#include "strobe/rdg/types/rendering_info.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/attachment_load_op.hpp"
#include "strobe/rhi/types/attachment_store_op.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rdg {

class CommandBuffer {
public:
  friend rdg::Timepoint submit(CommandBuffer cmd) noexcept;
  friend class CommandPool;

  // CommandBuffer() : m_runtime(flow::resource<rhi::CommandBuffer>()) {
  //   // special op: begin region.
  //   //    -> somehow populate m_runtime.
  //   flow::construct(m_runtime,
  //                   flow::intrinsic("strobe.rdg", "begin_cmd_region", 1));
  // }

  void dispatch(flow::Resource<uint32_t> wgX, flow::Resource<uint32_t> wgY,
                flow::Resource<uint32_t> wgZ) {
    flow::op(flow::read_write(m_runtime), flow::read(wgX), flow::read(wgY),
             flow::read(wgZ), flow::serialize(m_pool),
             [](rhi::CommandBuffer &cmd, const uint32_t &wgX,
                const uint32_t &wgY,
                const uint32_t &wgZ) noexcept { cmd.dispatch(wgX, wgY, wgZ); });
  }

  void begin_rendering(const RenderingInfo &info) {

    // resources.
    SmallVector<flow::Resource<rhi::ImageView>> colorAttachmentViews{};
    colorAttachmentViews.reserve(info.colorAttachments.size());
    SmallVector<flow::Resource<rhi::ImageView>> colorAttachmentResolveViews;
    colorAttachmentResolveViews.reserve(info.colorAttachments.size());
    SmallVector<flow::Resource<rhi::ClearValue>> colorAttachmentClearValues;
    colorAttachmentClearValues.reserve(info.colorAttachments.size());

    SmallVector<AttachmentImmediates> colorAttachmentImmediates{};
    colorAttachmentImmediates.reserve(info.colorAttachments.size());

    for (const Attachment &att : info.colorAttachments) {
      colorAttachmentViews.push_back(att.view.m_runtime);
      if (att.resolveView.has_value()) {
        colorAttachmentResolveViews.push_back(att.resolveView->m_runtime);
      } else {
        colorAttachmentResolveViews.push_back(flow::null_resource);
      }
      colorAttachmentImmediates.push_back(AttachmentImmediates{
          .layout = att.layout,
          .loadOp = att.loadOp,
          .storeOp = att.storeOp,
          .resolveMode = att.resolveMode,
      });

      if (att.clearValue.has_value()) {
        colorAttachmentClearValues.push_back(att.clearValue->runtime());
      }

      if (att.loadOp != rhi::AttachmentLoadOp::load) {
        if (!info.renderArea) {
          intr_image_discard(att.view.image());
        }
      } else {
        intr_image_access(att.view.image(),
                          rhi::PipelineStage::color_attachment_output,
                          rhi::Access::color_attachment_read, att.layout);
      }

      if (att.layout != rhi::ImageLayout::read_only) {
        intr_image_access(att.view.image(),
                          rhi::PipelineStage::color_attachment_output,
                          rhi::Access::color_attachment_write, att.layout);
      }

      if (att.resolveView.has_value()) {
        intr_image_access(att.resolveView->image(),
                          rhi::PipelineStage::color_attachment_output,
                          rhi::Access::color_attachment_write,
                          rhi::ImageLayout::general);
      }

      if (!info.renderArea &&
          att.storeOp == rhi::AttachmentStoreOp::dont_care) {
        m_state->discardAfterRendering.push_back(att.view.image());
      }
    }

    flow::Resource<rhi::ImageView> depthAttachmentView;
    flow::Resource<rhi::ImageView> depthAttachmentResolveView =
        flow::null_resource;
    flow::Resource<rhi::ClearValue> depthAttachmentClearValue =
        flow::null_resource;
    std::optional<AttachmentImmediates> depthAttachmentImmediates;
    if (info.depthAttachment.has_value()) {
      const auto &att = info.depthAttachment.value();
      depthAttachmentView = att.view.m_runtime;
      if (att.resolveView.has_value()) {
        depthAttachmentResolveView = att.resolveView->m_runtime;
      }
      depthAttachmentImmediates.emplace(AttachmentImmediates{
          .layout = att.layout,
          .loadOp = att.loadOp,
          .storeOp = att.storeOp,
          .resolveMode = att.resolveMode,
      });
      if (att.clearValue.has_value()) {
        depthAttachmentClearValue = att.clearValue->runtime();
      }

      if (att.loadOp != rhi::AttachmentLoadOp::load) {
        if (!info.renderArea) {
          intr_image_discard(att.view.image());
        }
      } else {
        intr_image_access(att.view.image(),
                          rhi::PipelineStage::early_fragment_tests |
                              rhi::PipelineStage::late_fragment_tests,
                          rhi::Access::depth_stencil_attachment_read,
                          att.layout);
      }

      if (att.layout != rhi::ImageLayout::read_only) {
        intr_image_access(att.view.image(),
                          rhi::PipelineStage::early_fragment_tests |
                              rhi::PipelineStage::late_fragment_tests,
                          rhi::Access::depth_stencil_attachment_write,
                          att.layout);
      }

      if (att.resolveView.has_value()) {
        intr_image_access(att.resolveView->image(),
                          rhi::PipelineStage::early_fragment_tests |
                              rhi::PipelineStage::late_fragment_tests,
                          rhi::Access::depth_stencil_attachment_write,
                          rhi::ImageLayout::general);
      }

      if (!info.renderArea &&
          att.storeOp == rhi::AttachmentStoreOp::dont_care) {
        m_state->discardAfterRendering.push_back(att.view.image());
      }
    }

    flow::Resource<rhi::ImageView> stencilAttachmentView;
    flow::Resource<rhi::ImageView> stencilAttachmentResolveView =
        flow::null_resource;
    flow::Resource<rhi::ClearValue> stencilAttachmentClearValue =
        flow::null_resource;
    std::optional<AttachmentImmediates> stencilAttachmentImmediates;
    if (info.stencilAttachment.has_value()) {
      const auto &att = info.stencilAttachment.value();
      stencilAttachmentView = att.view.m_runtime;
      if (att.resolveView.has_value()) {
        stencilAttachmentResolveView = att.resolveView->m_runtime;
      }
      stencilAttachmentImmediates.emplace(AttachmentImmediates{
          .layout = att.layout,
          .loadOp = att.loadOp,
          .storeOp = att.storeOp,
          .resolveMode = att.resolveMode,
      });
      if (att.clearValue.has_value()) {
        depthAttachmentClearValue = att.clearValue->runtime();
      }

      if (att.loadOp != rhi::AttachmentLoadOp::load) {
        if (!info.renderArea) {
          intr_image_discard(att.view.image());
        }
      } else {
        intr_image_access(att.view.image(),
                          rhi::PipelineStage::early_fragment_tests |
                              rhi::PipelineStage::late_fragment_tests,
                          rhi::Access::depth_stencil_attachment_read,
                          att.layout);
      }

      if (att.layout != rhi::ImageLayout::read_only) {
        intr_image_access(att.view.image(),
                          rhi::PipelineStage::early_fragment_tests |
                              rhi::PipelineStage::late_fragment_tests,
                          rhi::Access::depth_stencil_attachment_write,
                          att.layout);
      }

      if (att.resolveView.has_value()) {
        intr_image_access(att.resolveView->image(),
                          rhi::PipelineStage::early_fragment_tests |
                              rhi::PipelineStage::late_fragment_tests,
                          rhi::Access::depth_stencil_attachment_write,
                          rhi::ImageLayout::general);
      }

      if (!info.renderArea &&
          att.storeOp == rhi::AttachmentStoreOp::dont_care) {
        m_state->discardAfterRendering.push_back(att.view.image());
      }
    }

    flow::op(
        flow::intrinsic("strobe.rdg", "cmd_begin_rendering", 1)
            .immediate_span<AttachmentImmediates>(colorAttachmentImmediates) //
            .optional_immediate<AttachmentImmediates>(                       //
                depthAttachmentImmediates)                                   //
            .optional_immediate<AttachmentImmediates>(                       //
                stencilAttachmentImmediates)                                 //
            .immediate<uint32_t>(info.layerCount)                            //
            .immediate<uint32_t>(info.viewMask),                             //
        flow::read_write(m_runtime),                                         //
        flow::read_span<rhi::ImageView>(colorAttachmentViews),               //
        flow::read_span<rhi::ImageView>(colorAttachmentResolveViews),        //
        flow::read_span<rhi::ClearValue>(colorAttachmentClearValues),        //
        flow::read(depthAttachmentView),                                     //
        flow::read(depthAttachmentResolveView),                              //
        flow::read(depthAttachmentClearValue),
        flow::read(stencilAttachmentView),        //
        flow::read(stencilAttachmentResolveView), //
        flow::read(stencilAttachmentClearValue),
        flow::read(info.renderArea ? info.renderArea->runtime()    //
                                   : flow::Resource<rhi::Rect>{}), //
        flow::serialize(m_pool));                                  //
  }

  void end_rendering() {
    flow::op(flow::intrinsic("strobe.rdg", "cmd_end_rendering", 1),
             flow::read_write(m_runtime), flow::serialize(m_pool));
    for (const auto &image : m_state->discardAfterRendering) {
      intr_image_discard(image);
    }
    m_state->discardAfterRendering.clear();
  }

  void present(SwapchainImage target) noexcept {
    intr_image_access(target.image(), rhi::PipelineStage::none,
                      rhi::Access::none, rhi::ImageLayout::present);
    flow::op(flow::intrinsic("strobe.rdg", "present", 1),
             flow::consume(target.m_runtime),
             flow::read(target.m_image.m_runtime), flow::serialize(m_runtime));
  }

private:
  void intr_image_access(Image image, rhi::PipelineStage stage,
                         rhi::Access access, rhi::ImageLayout layout) {
    flow::op(flow::intrinsic("strobe.rdg", "image-access", 1)
                 .immediate(stage)        //
                 .immediate(access)       //
                 .immediate(layout),      //
             flow::read_write(m_runtime), //
             flow::read(image.m_runtime), //
             flow::serialize(m_pool));    //
  }

  void intr_image_discard(Image image) {
    flow::op(flow::intrinsic("strobe.rdg", "cmd-discard-image", 1),
             flow::read_write(m_runtime), flow::read_write(image.m_runtime),
             flow::serialize(m_pool));
  }

  void intr_buffer_access(Buffer buffer, rhi::PipelineStage stage,
                          rhi::Access access) {
    flow::op(flow::intrinsic("strobe.rdg", "cmd-buffer-access", 1)
                 .immediate(stage)         //
                 .immediate(access),       //
             flow::read_write(m_runtime),  //
             flow::read(buffer.m_runtime), //
             flow::serialize(m_pool));     //
  }

  explicit CommandBuffer(flow::Resource<rhi::CommandBuffer> runtime,
                         flow::Resource<rhi::CommandPool> pool) noexcept
      : m_runtime(std::move(runtime)), m_pool(std::move(pool)),
        m_state(make_rc<RecordingState>()) {}

private:
  struct RecordingState {
    Vector<Image> discardAfterRendering;
  };

  flow::Resource<rhi::CommandBuffer> m_runtime;
  flow::Resource<rhi::CommandPool> m_pool;
  Rc<RecordingState> m_state;
};

inline rdg::Timepoint submit(CommandBuffer cmd) noexcept {
  flow::op(flow::OpMetadata{"end-cmd"}, flow::read_write(cmd.m_runtime),
           flow::serialize(cmd.m_pool), [](rhi::CommandBuffer &cmd) noexcept {
             ZoneScopedN("cmd-end-rendering");
             cmd.end();
           });

  auto timepoint = flow::resource<rhi::Timepoint>();
  flow::construct(timepoint, flow::intrinsic("strobe.rdg", "submit", 1),
                  flow::consume(cmd.m_runtime), flow::serialize(cmd.m_pool));
  return rdg::Timepoint{timepoint};
}

} // namespace strobe::rdg
