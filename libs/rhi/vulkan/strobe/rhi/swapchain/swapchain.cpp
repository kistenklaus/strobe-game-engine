#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_impl.hpp"
#include <limits>

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
  if (impl->generation.suboptimal()) {
    impl->recreate();
  }
  constexpr uint64_t initialTimeout = 1'000;     // 1 us
  constexpr uint64_t maximumTimeout = 1'000'000; // 1 ms
  uint64_t timeout = initialTimeout;
  while (true) {
    auto [image, timedOut] = impl->generation.acquire(timeout);
    if (image) {
      return image;
    }
    if (timedOut) {
      if (timeout < maximumTimeout) {
        timeout = std::min(timeout * 2, maximumTimeout);
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
      }
      continue;
    }
    if (!impl->recreate()) {
      return {}; // minimized
    }
    timeout = initialTimeout;
  }
}
void Swapchain::resize_hint(uvec2 size) noexcept {
  ZoneScopedN("Swapchain::resize");
  auto *impl = void_handle_ptr<SwapchainImpl>(m_handle);
  impl->resize(size);
}

} // namespace strobe::rhi
