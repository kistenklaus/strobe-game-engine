#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
class Swapchain : public Object<Swapchain> {
  friend class Object<Swapchain>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit Swapchain(void *handle) noexcept : Object(handle) {}
  SwapchainImage acquire();
};

} // namespace strobe::rhi
