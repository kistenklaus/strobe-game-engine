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
  Image image() const noexcept { return m_image; }

  ImageView view() const noexcept { return m_view; }

private:
  SwapchainImage(flow::Resource<swapchain_image_runtime> runtime,
                 flow::Resource<rhi::Image> image,
                 flow::Resource<rhi::ImageView> view) noexcept
      : m_runtime(std::move(runtime)), m_image(std::move(image)),
        m_view(std::move(view), m_image.m_runtime) {}
  flow::Resource<swapchain_image_runtime> m_runtime;
  Image m_image;
  ImageView m_view;
};

inline SwapchainImage acquire(Swapchain swapchain) {

  auto swapchain_image = flow::resource<swapchain_image_runtime>();
  flow::construct(
      swapchain_image,
      flow::external_op("swapchain", "acquire", 1,
                        flow::OpMetadata("acquire", "acquire swapchain image",
                                         flow::OpSemantics::side_effect)),
      flow::read_write(swapchain.m_runtime));

  auto image = flow::project<rhi::Image>(
      swapchain_image, "image", flow::ProjectionKind::disjoint,
      flow::external_projector("swapchain", "image", 1));

  auto view = flow::project<rhi::ImageView>(
      swapchain_image, "view", flow::ProjectionKind::disjoint,
      flow::external_projector("swapchain", "view", 1));

  auto result = SwapchainImage{swapchain_image, image, view};
  intr::decl_image_state(result.image(), rhi::PipelineStage::none,
                         rhi::Access::none, rhi::ImageLayout::undefined);
  return result;
}

} // namespace strobe::rdg
