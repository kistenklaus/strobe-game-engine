#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"

namespace strobe::rhi {


SwapchainImage::SwapchainImage(SwapchainImage &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

SwapchainImage &SwapchainImage::operator=(SwapchainImage &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<SwapchainImageImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

SwapchainImage::~SwapchainImage() noexcept {
  unpin_void_handle<SwapchainImageImpl>(m_handle);
}

const Image &SwapchainImage::image() const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  auto *gen =
      void_handle_ptr<SwapchainGenerationImpl>(impl->generation.m_handle);
  return gen->frames[impl->index].image;
}

const ImageView &SwapchainImage::view() const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  auto *gen =
      void_handle_ptr<SwapchainGenerationImpl>(impl->generation.m_handle);
  return gen->frames[impl->index].view;
}

const uvec2 SwapchainImage::extent() const noexcept {
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  auto *gen =
      void_handle_ptr<SwapchainGenerationImpl>(impl->generation.m_handle);
  return gen->extent;
}

} // namespace strobe::rhi
