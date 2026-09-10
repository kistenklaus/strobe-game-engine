#pragma once

#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
class SwapchainImage : public Object<SwapchainImage> {
public:
  explicit SwapchainImage(void *handle) noexcept : Object(handle) {}
  SwapchainImage() noexcept : Object(nullptr) {}
  SwapchainImage(const SwapchainImage &) = delete;
  SwapchainImage(SwapchainImage &&) noexcept;
  SwapchainImage &operator=(const SwapchainImage &) = delete;
  SwapchainImage &operator=(SwapchainImage &&) noexcept;
  ~SwapchainImage() noexcept;


  const Image &image() const noexcept;
  const ImageView &view() const noexcept;
  const uvec2 extent() const noexcept;
};

} // namespace strobe::rhi
