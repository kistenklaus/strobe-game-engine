#pragma once

#include "hdr.hpp"
#include "mbuffer.hpp"
#include "radiance_buffer.hpp"
#include "strobe/core/containers/boxed_array.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "vbuffer.hpp"
#include <cstdint>

struct DeferredRenderer {
private:
  struct Frame {
    strobe::uvec2 gbuffer_extent;
    VBuffer vbuffer{};
    MBuffer mbuffer{};
    RBuffer rbuffer{};
    HDR hdr{};

    void resize(const strobe::rhi::Device &device,
                strobe::uvec2 target_extent) noexcept {
      if (target_extent != gbuffer_extent) {
        vbuffer = VBuffer::create(device, target_extent);
        mbuffer = MBuffer::create(device, target_extent);
        rbuffer = RBuffer::create(device, target_extent);
        hdr = HDR::create(device, target_extent);
        gbuffer_extent = target_extent;
      }
    }

    Frame() noexcept : gbuffer_extent{0, 0} {}
  };

public:
  explicit DeferredRenderer(strobe::rhi::Device device,
                            uint32_t framesInFlight) noexcept
      : m_device(std::move(device)), m_frames(framesInFlight) {
    //
  }

  void geometry_pass(strobe::rhi::CommandBuffer &cmd, Frame &frame) noexcept {

    cmd.transition_image(frame.vbuffer.visibility,
                         strobe::rhi::ImageLayout::undefined,
                         strobe::rhi::ImageLayout::attachment);
    cmd.transition_image(frame.mbuffer.motion,
                         strobe::rhi::ImageLayout::undefined,
                         strobe::rhi::ImageLayout::attachment);
    cmd.transition_image(frame.vbuffer.depth,
                         strobe::rhi::ImageLayout::undefined,
                         strobe::rhi::ImageLayout::attachment);

    std::array<strobe::rhi::Attachment, 2> colorAtt{};

    colorAtt[0] = {
        .view = frame.vbuffer.visibilityView,
        .layout = strobe::rhi::ImageLayout::attachment,
        .loadOp = strobe::rhi::AttachmentLoadOp::clear,
    };
    colorAtt[1] = {
        .view = frame.mbuffer.motionView,
        .layout = strobe::rhi::ImageLayout::attachment,
        .loadOp = strobe::rhi::AttachmentLoadOp::dont_care,
    };

    strobe::rhi::Attachment depthAtt{
        .view = frame.vbuffer.depthView,
        .layout = strobe::rhi::ImageLayout::attachment,
        .loadOp = strobe::rhi::AttachmentLoadOp::clear,
    };

    cmd.begin_rendering({
        .colorAttachments = colorAtt,
        .depthAttachment = depthAtt,
    });
    // draw geometry.
    cmd.end_rendering();
  }

  void lighting_pass(strobe::rhi::CommandBuffer &cmd, Frame &frame) noexcept {

    cmd.transition_image(frame.rbuffer.radiance,
                         strobe::rhi::ImageLayout::undefined,
                         strobe::rhi::ImageLayout::attachment);
    cmd.transition_image(frame.vbuffer.depth,
                         strobe::rhi::ImageLayout::attachment,
                         strobe::rhi::ImageLayout::read_only);

    strobe::rhi::Attachment radianceAtt{
        .view = frame.rbuffer.radianceView,
        .layout = strobe::rhi::ImageLayout::attachment,
        .loadOp = strobe::rhi::AttachmentLoadOp::clear,
    };

    strobe::rhi::Attachment depthAtt{
        .view = frame.vbuffer.depthView,
        .layout = strobe::rhi::ImageLayout::read_only,
        .loadOp = strobe::rhi::AttachmentLoadOp::load,
        .storeOp = strobe::rhi::AttachmentStoreOp::none,
    };

    cmd.begin_rendering({
        .colorAttachments = &radianceAtt,
        .depthAttachment = depthAtt,
    });
    // draw lights.
    cmd.end_rendering();
  }

  void forward(strobe::rhi::CommandBuffer &cmd, Frame &frame) noexcept {
    strobe::rhi::Attachment radianceAtt{
        .view = frame.rbuffer.radianceView,
        .layout = strobe::rhi::ImageLayout::attachment,
        .loadOp = strobe::rhi::AttachmentLoadOp::load,
    };

    strobe::rhi::Attachment depthAtt{
        .view = frame.vbuffer.depthView,
        .layout = strobe::rhi::ImageLayout::read_only,
        .loadOp = strobe::rhi::AttachmentLoadOp::load,
        .storeOp = strobe::rhi::AttachmentStoreOp::none,
    };

    cmd.begin_rendering({
        .colorAttachments = &radianceAtt,
        .depthAttachment = depthAtt,
    });
    // draw lights.
    cmd.end_rendering();
  }

  void taa(strobe::rhi::CommandBuffer &cmd, Frame &frame) noexcept {
    cmd.transition_image(frame.hdr.color, strobe::rhi::ImageLayout::undefined,
                         strobe::rhi::ImageLayout::general);
    // read radiance + motion write hdr.

    // compute dispatch
  }

  void tone_mapping(strobe::rhi::CommandBuffer &cmd, Frame &frame,
                    const strobe::rhi::Image &target) {
    cmd.transition_image(target, strobe::rhi::ImageLayout::undefined,
                         strobe::rhi::ImageLayout::general);
    // read hdr write target.

    // compute dispatch
  }

  void render(strobe::rhi::CommandBuffer &cmd, const strobe::rhi::Image &target,
              uint32_t frameIndex) {
    auto &frame = m_frames[frameIndex];
    frame.resize(m_device, target.extent().xy());

    geometry_pass(cmd, frame);

    lighting_pass(cmd, frame);

    taa(cmd, frame);

    tone_mapping(cmd, frame, target);

    cmd.transition_image(target, strobe::rhi::ImageLayout::general,
                         strobe::rhi::ImageLayout::present);
  }

private:
  strobe::rhi::Device m_device;
  strobe::BoxedArray<Frame> m_frames;
};
