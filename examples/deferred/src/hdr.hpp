#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"

struct HDR {

  static HDR create(const strobe::rhi::Device &device,
                    strobe::uvec2 extent) noexcept {
    HDR hdr;
    hdr.color = device.create_image({
        .type = strobe::rhi::ImageType::image_2d,
        .format = strobe::rhi::Format::rgba16_float,
        .extent = {extent.x(), extent.y(), 1},
        .imageUsage = strobe::rhi::ImageUsage::storage,
    });
    hdr.colorStorageDescriptor = device.create_storage_image_descriptor({
        .image = hdr.color,
        .layout = strobe::rhi::ImageLayout::general,
        .subresource = {.aspect = strobe::rhi::ImageAspect::color},
    });
    return hdr;
  }

  strobe::rhi::Image color;
  strobe::rhi::ResourceDescriptor colorStorageDescriptor;
};
