#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image.hpp"
struct MBuffer {

  static MBuffer create(const strobe::rhi::Device &device,
                        strobe::uvec2 extent) {
    MBuffer mbuf;
    mbuf.motion = device.create_image({
        .type = strobe::rhi::ImageType::image_2d,
        .format = strobe::rhi::Format::rg16_float,
        .extent = {extent.x(), extent.y(), 1},
        .imageUsage = strobe::rhi::ImageUsage::color_attachment |
                      strobe::rhi::ImageUsage::storage,
    });
    mbuf.motionView = device.create_image_view(
        mbuf.motion, {
                         .type = strobe::rhi::ImageViewType::image_2d,
                         .format = strobe::rhi::Format::rg16_float,
                         .range = {.aspect = strobe::rhi::ImageAspect::color},
                     });
    return mbuf;
  }

  strobe::rhi::Image motion;
  strobe::rhi::ImageView motionView;
};
