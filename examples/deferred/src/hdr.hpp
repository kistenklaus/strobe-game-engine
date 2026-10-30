#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image.hpp"

struct HDR {

  static HDR create(const strobe::rhi::Device &device,
                    strobe::uvec2 extent) noexcept {
    HDR hdr;
    hdr.color = device.create_image({
        .type = strobe::rhi::ImageType::image_2d,
        .format = strobe::rhi::Format::rgba16_float,
        .extent = {extent.x(), extent.y(), 1},
    });
    return hdr;
  }

  strobe::rhi::Image color;
};
