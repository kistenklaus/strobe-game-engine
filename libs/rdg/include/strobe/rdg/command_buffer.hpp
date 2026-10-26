#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/flow/access.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/resource.hpp"
#include "strobe/rdg/buffer.hpp"
#include "strobe/rdg/swapchain.hpp"
#include "strobe/rdg/types/rendering_info.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rdg {

class CommandBuffer {
public:
  friend void submit(CommandBuffer cmd) noexcept;
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

    SmallVector<flow::Resource<rhi::ImageView>> colorAttachmentViews{};
    colorAttachmentViews.reserve(info.colorAttachments.size());
    SmallVector<rhi::AttachmentLoadOp> colorAttachmentLoadOps{};
    colorAttachmentLoadOps.reserve(info.colorAttachments.size());
    SmallVector<rhi::AttachmentStoreOp> colorAttachmentStoreOps{};
    colorAttachmentStoreOps.reserve(info.colorAttachments.size());
    SmallVector<rhi::ClearValue> colorAttachmentClearValues{};
    colorAttachmentClearValues.reserve(info.colorAttachments.size());
    SmallVector<rhi::ResolveMode> colorAttachmentResolveModes{};
    colorAttachmentResolveModes.reserve(info.colorAttachments.size());
    SmallVector<flow::Resource<rhi::ImageView>> colorAttachmentImageViews; // <- this need some form of optional span type!


    flow::op(flow::intrinsic("strobe.rdg", "cmd_begin_rendering", 1)
                 .immediate<uint32_t>(info.layoutCount) //
                 .immediate<uint32_t>(info.viewMask),   //
             flow::read_write(m_runtime),               //
             flow::serialize(m_pool));                  //
  }

  void end_rendering() {
    flow::op(flow::intrinsic("strobe.rdg", "cmd_end_rendering", 1),
             flow::read_write(m_runtime), flow::serialize(m_pool));
  }

  void present(SwapchainImage target) noexcept {
    intr_image_access(target.image(), rhi::PipelineStage::none,
                      rhi::Access::none, rhi::ImageLayout::present);
    flow::op(flow::intrinsic("strobe.rdg", "present", 1),
             flow::consume(target.m_runtime), flow::read(target.m_image),
             flow::serialize(m_runtime));
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

  void intr_decl_image_state(Image image, rhi::PipelineStage stage,
                             rhi::Access access, rhi::ImageLayout layout) {
    flow::op(flow::intrinsic("strobe.rdg", "cmd-decl-image-state", 1)
                 .immediate(stage)              //
                 .immediate(access)             //
                 .immediate(layout),            //
             flow::read_write(m_runtime),       //
             flow::read_write(image.m_runtime), //
             flow::serialize(m_pool));          //
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

  void intr_decl_buffer_state(Buffer buffer, rhi::PipelineStage stage,
                              rhi::Access access) {
    flow::op(flow::intrinsic("strobe.rdg", "cmd-decl-buffer-state", 1)
                 .immediate(stage)               //
                 .immediate(access),             //
             flow::read_write(m_runtime),        //
             flow::read_write(buffer.m_runtime), //
             flow::serialize(m_pool));           //
  }

  explicit CommandBuffer(flow::Resource<rhi::CommandBuffer> runtime,
                         flow::Resource<rhi::CommandPool> pool) noexcept
      : m_runtime(std::move(runtime)), m_pool(std::move(pool)) {}

private:
  flow::Resource<rhi::CommandBuffer> m_runtime;
  flow::Resource<rhi::CommandPool> m_pool;
};

inline void submit(CommandBuffer cmd) noexcept {
  flow::op(flow::OpMetadata{"end-cmd"}, flow::read_write(cmd.m_runtime),
           flow::serialize(cmd.m_pool),
           [](rhi::CommandBuffer &cmd) noexcept { cmd.end(); });

  flow::op(flow::intrinsic("strobe.rdg", "submit", 1),
           flow::consume(cmd.m_runtime), flow::serialize(cmd.m_pool));
}

} // namespace strobe::rdg
