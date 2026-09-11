#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Swapchain
 * Defined in header <strobe/rhi/rhi.hpp>
 * \code{.cpp}
 * class Swapchain : public Object<Swapchain>;
 * \endcode
 */
class Swapchain : public Object<Swapchain> {
  friend class Object<Swapchain>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  SwapchainImage acquire();
};

} // namespace strobe::rhi
