#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"

namespace strobe::rhi {

void SwapchainImage::pin(void *handle) noexcept {
  pin_void_handle<SwapchainImageImpl>(handle);
}
void SwapchainImage::unpin(void *handle) noexcept {
  unpin_void_handle<SwapchainImageImpl>(handle);
}

const Image &SwapchainImage::image() const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  auto *gen = object_handle_ptr<SwapchainGenerationImpl>(impl->generation);
  return gen->frames[impl->index].image;
}

const ImageView &SwapchainImage::view() const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  auto *gen = object_handle_ptr<SwapchainGenerationImpl>(impl->generation);
  return gen->frames[impl->index].view;
}

const uvec2 SwapchainImage::extent() const noexcept {
  auto *impl = void_handle_ptr<SwapchainImageImpl>(m_handle);
  auto *gen = object_handle_ptr<SwapchainGenerationImpl>(impl->generation);
  return gen->extent;
}

} // namespace strobe::rhi
