#pragma once

#include "strobe/flow/access.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/rdg/image.hpp"
#include "strobe/rdg/intrinsics.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"

namespace strobe::rdg {

using swapchain_runtime = void *;
using swapchain_image_runtime = void *;

class SwapchainImage;

class Swapchain {
  friend SwapchainImage acquire(Swapchain swapchain);

public:
  Swapchain(std::string_view name) noexcept
      : m_runtime(flow::external<swapchain_runtime>(name)) {}

private:
  flow::Resource<swapchain_runtime> m_runtime;
};

class SwapchainImage {
  friend class CommandBuffer;
  friend SwapchainImage acquire(Swapchain swapchain);

public:
  Image image() const noexcept { return Image{m_image}; }

  ImageView view() const noexcept { return ImageView{m_view, m_image}; }

private:
  SwapchainImage(flow::Resource<swapchain_image_runtime> runtime,
                 flow::Resource<rhi::Image> image,
                 flow::Resource<rhi::ImageView> view) noexcept
      : m_runtime(std::move(runtime)), m_image(std::move(image)),
        m_view(std::move(view)) {}
  flow::Resource<swapchain_image_runtime> m_runtime;
  flow::Resource<rhi::Image> m_image;
  flow::Resource<rhi::ImageView> m_view;
};

inline SwapchainImage acquire(Swapchain swapchain) {

  auto swapchain_image = flow::resource<swapchain_image_runtime>();
  flow::construct(
      swapchain_image,
      flow::external_op("swapchain", "acquire", 1,
                        flow::OpMetadata("acquire", "acquire swapchain image",
                                         flow::OpSemantics::side_effect)),
      flow::read_write(swapchain.m_runtime));

  auto image = flow::resource<rhi::Image>();
  flow::construct(
      image,
      flow::external_op(
          "swapchain", "get_image", 1,
          flow::OpMetadata("get_image", "get image of swapchain image")),
      flow::read(swapchain_image));
  intr::decl_image_state(image, rhi::PipelineStage::none, rhi::Access::none,
                         rhi::ImageLayout::undefined);

  auto view = flow::resource<rhi::ImageView>();
  flow::construct(
      view,
      flow::external_op(
          "swapchain", "get_view", 1,
          flow::OpMetadata{"get_view", "get image view of swapchain image"}),
      flow::read(swapchain_image));

  return SwapchainImage{swapchain_image, image, view};
}

} // namespace strobe::rdg
