#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"

struct VBuffer {

  static VBuffer create(const strobe::rhi::Device &device,
                        strobe::uvec2 extent) {
    VBuffer vbuffer;
    vbuffer.depth = device.create_image({
        .type = strobe::rhi::ImageType::image_2d,
        .format = strobe::rhi::Format::d32_float,
        .extent = {extent.x(), extent.y(), 1},
        .imageUsage = strobe::rhi::ImageUsage::depth_stencil |
                      strobe::rhi::ImageUsage::sampled,
    });
    vbuffer.depthView = device.create_image_view(
        vbuffer.depth, {
                           .type = strobe::rhi::ImageViewType::image_2d,
                           .format = strobe::rhi::Format::d32_float,
                           .range = {.aspect = strobe::rhi::ImageAspect::depth},
                       });
    vbuffer.visibilityStorage = device.create_storage_image_descriptor({
        .image = vbuffer.depth,
        .layout = strobe::rhi::ImageLayout::general,
        .subresource = {.aspect = strobe::rhi::ImageAspect::depth},
    });

    vbuffer.visibility = device.create_image({
        .type = strobe::rhi::ImageType::image_2d,
        .format = strobe::rhi::Format::r32_uint,
        .extent = {extent.x(), extent.y(), 1},
        .imageUsage = strobe::rhi::ImageUsage::color_attachment,
    });
    vbuffer.visibilityView = device.create_image_view(
        vbuffer.visibility,
        {
            .type = strobe::rhi::ImageViewType::image_2d,
            .format = strobe::rhi::Format::r32_uint,
            .range = {.aspect = strobe::rhi::ImageAspect::color},
        });
    vbuffer.visibilityStorage = device.create_storage_image_descriptor({
        .image = vbuffer.visibility,
        .layout = strobe::rhi::ImageLayout::general,
        .subresource = {.aspect = strobe::rhi::ImageAspect::color},
    });
    return vbuffer;
  }

  strobe::rhi::Image depth;
  strobe::rhi::ImageView depthView;
  strobe::rhi::ResourceDescriptor depthStorage;
  strobe::rhi::Image visibility;
  strobe::rhi::ImageView visibilityView;
  strobe::rhi::ResourceDescriptor visibilityStorage;
};
