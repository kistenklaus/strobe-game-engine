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
  friend class Object<SwapchainImage>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit SwapchainImage(void *handle) noexcept : Object(handle) {}


  const Image &image() const noexcept;
  const ImageView &view() const noexcept;
  const uvec2 extent() const noexcept;
};

} // namespace strobe::rhi
