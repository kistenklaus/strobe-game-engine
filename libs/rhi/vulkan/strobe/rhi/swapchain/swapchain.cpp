#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"

namespace strobe::rhi {

void Swapchain::pin(void *handle) noexcept {
  pin_void_handle<SwapchainImpl>(handle);
}

void Swapchain::unpin(void *handle) noexcept {
  unpin_void_handle<SwapchainImpl>(handle);
}

SwapchainImage Swapchain::acquire() {
  ZoneScopedN("Swapchain::acquire");
  auto *impl = void_handle_ptr<SwapchainImpl>(m_handle);
  if (!impl->generation && !impl->recreate()) {
    return {};
  }
  assert(impl->generation);
  // if (impl->generation.suboptimal()) {
  //   impl->recreate();
  // }
  while (true) {
    SwapchainImage image = impl->generation.acquire();
    if (image) {
      return image;
    }
    if (!impl->recreate()) {
      return {}; // minimized
    }
  }
}

void Swapchain::resize_hint(uvec2 size) noexcept {
  ZoneScopedN("Swapchain::resize");
  auto* impl = void_handle_ptr<SwapchainImpl>(m_handle);
  impl->resize(size);
}

} // namespace strobe::rhi
