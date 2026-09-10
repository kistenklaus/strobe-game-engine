#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief todo
 */
class Swapchain : public Object<Swapchain> {
public:
  explicit Swapchain(void *handle) noexcept : Object(handle) {}
  Swapchain() noexcept : Object(nullptr) {}
  Swapchain(const Swapchain &) noexcept;
  Swapchain(Swapchain &&) noexcept;
  Swapchain &operator=(const Swapchain &) noexcept;
  Swapchain &operator=(Swapchain &&) noexcept;
  ~Swapchain() noexcept;

  SwapchainImage acquire();
};

} // namespace strobe::rhi
