#pragma once

#include "strobe/flow/resource.hpp"
#include "strobe/flow/resource_id.hpp"
#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"

namespace strobe::rdg {

class Image;

namespace intr {

inline void decl_image_state(Image image, rhi::PipelineStage stage,
                             rhi::Access access, rhi::ImageLayout layout);

}

class Image {
public:
  friend class SwapchainImage;
  friend class CommandBuffer;
  friend class ImageView;
  friend void intr::decl_image_state(Image image, rhi::PipelineStage stage,
                                     rhi::Access access,
                                     rhi::ImageLayout layout);


private:
  explicit Image(flow::Resource<rhi::Image> runtime) noexcept
      : m_runtime(std::move(runtime)) {}
  flow::Resource<rhi::Image> m_runtime;
};

class ImageView {
public:
  friend class SwapchainImage;
  friend class CommandBuffer;

  Image image() const { return Image{m_image}; }

private:
  explicit ImageView(flow::Resource<rhi::ImageView> runtime,
                     flow::Resource<rhi::Image> image) noexcept
      : m_runtime(std::move(runtime)), m_image(std::move(image)) {}
  flow::Resource<rhi::ImageView> m_runtime;
  flow::Resource<rhi::Image> m_image;
};

} // namespace strobe::rdg
