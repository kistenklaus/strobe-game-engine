#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"

struct RBuffer {

  static RBuffer create(const strobe::rhi::Device &device,
                        strobe::uvec2 extent) {
    RBuffer rbuf;
    rbuf.radiance = device.create_image({
        .type = strobe::rhi::ImageType::image_2d,
        .format = strobe::rhi::Format::rgba16_float,
        .extent = {extent.x(), extent.y(), 1},
        .imageUsage = strobe::rhi::ImageUsage::color_attachment |
                      strobe::rhi::ImageUsage::storage,
    });
    rbuf.radianceView = device.create_image_view(
        rbuf.radiance, {
                           .type = strobe::rhi::ImageViewType::image_2d,
                           .format = strobe::rhi::Format::rgba16_float,
                           .range = {.aspect = strobe::rhi::ImageAspect::color},
                       });
    rbuf.radianceStorage = device.create_storage_image_descriptor({
        .image = rbuf.radiance,
        .layout = strobe::rhi::ImageLayout::general,
        .subresource = {.aspect = strobe::rhi::ImageAspect::color},
    });
    return rbuf;
  }

  strobe::rhi::Image radiance;
  strobe::rhi::ImageView radianceView;
  strobe::rhi::ResourceDescriptor radianceStorage;
};
