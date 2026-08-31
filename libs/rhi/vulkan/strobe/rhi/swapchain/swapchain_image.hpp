#pragma once

#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi {

class SwapchainImage : Object<SwapchainImage> {
public:
  explicit SwapchainImage(void *handle) noexcept : Object(handle) {}
  SwapchainImage() noexcept : Object(nullptr) {}
  SwapchainImage(const SwapchainImage &) noexcept;
  SwapchainImage(SwapchainImage &&) noexcept;
  SwapchainImage &operator=(const SwapchainImage &) noexcept;
  SwapchainImage &operator=(SwapchainImage &&) noexcept;
  ~SwapchainImage() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const SwapchainImage &lhs,
                         const SwapchainImage &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const SwapchainImage &lhs,
                         const SwapchainImage &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  const Image &image() const noexcept;
  const ImageView &view() const noexcept;
};

} // namespace strobe::rhi
