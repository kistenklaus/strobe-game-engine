#include "strobe/rhi/swapchain/swapchain_generation.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/object_factory.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/sync/fence_callback_flag.hpp"
#include "strobe/rhi/vulkan/swapchain.hpp"
#include <atomic>
#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

void SwapchainGeneration::pin(void *handle) noexcept {
  pin_void_handle<SwapchainGenerationImpl>(handle);
}

void SwapchainGeneration::unpin(void *handle) noexcept {
  unpin_void_handle<SwapchainGenerationImpl>(handle);
}

SwapchainImage SwapchainGeneration::acquire() {
  ZoneScopedN("swap/acquire");
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  vulkan::Context *ctx = impl->surface.ctx();
  BinarySemaphore imageAvailable = impl->semPool.allocate();
  uint32_t imageIndex;

  std::lock_guard lck{impl->mutex};
  const auto result = vulkan::acquire_next_swapchain_image(
      ctx, impl->swapchain,
      {
          .signalSemaphore = imageAvailable.signal(),
      },
      &imageIndex);

  switch (result) {
  case vulkan::SwapchainAcquireStatus::success: {
    impl->debugCounter.fetch_add(1, std::memory_order_relaxed);
    // this line somhow changes the allocator.
    impl->frames[imageIndex].imageAvailable = std::move(imageAvailable);
    auto alloc = impl->get_swapchain_image_handle_allocator();
    auto *ptr = make_void_handle<SwapchainImageImpl>(alloc, *this, imageIndex);
    return detail::make_object<SwapchainImage>(ptr);
  }
  case vulkan::SwapchainAcquireStatus::suboptimal:
    fmt::println("suboptimal");
    impl->debugCounter.fetch_add(1, std::memory_order_relaxed);
    impl->frames[imageIndex].imageAvailable = std::move(imageAvailable);
    impl->suboptimal = true;
    return detail::make_object<SwapchainImage>(
        make_void_handle<SwapchainImageImpl>(
            impl->get_swapchain_image_handle_allocator(), //
            *this, imageIndex));
  case vulkan::SwapchainAcquireStatus::out_of_date:
    return {};
  case vulkan::SwapchainAcquireStatus::timeout:
    vulkan_error(VK_TIMEOUT, "unexpected swapchain acquire result: {}",
                 impl->debugCounter.load());
  case vulkan::SwapchainAcquireStatus::not_ready:
    vulkan_error(VK_NOT_READY, "unexpected swapchain acquire result");
  }
  std::unreachable();
}

std::pair<BinarySemaphore, Fence>
SwapchainGeneration::present([[maybe_unused]] uint32_t index) {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  assert(impl);

  auto *presentFrame = static_cast<SwapchainPresentFrame *>(
      impl->get_present_frame_allocator().allocate(
          sizeof(SwapchainPresentFrame), alignof(SwapchainPresentFrame)));

  std::construct_at(presentFrame, *this, impl->semPool.allocate());

  Fence presentFence =
      impl->fencePool.allocate(presentFrame, [](void *ptr, FenceCallbackFlag) {
        auto *presentFrame = static_cast<SwapchainPresentFrame *>(ptr);
        // the generations own the backing allocator so we have to be careful
        // here.
        SwapchainGeneration generation = std::move(presentFrame->generation);
        std::destroy_at(presentFrame);
        object_handle_ptr<SwapchainGenerationImpl>(generation)
            ->get_present_frame_allocator()
            .deallocate(presentFrame);
      });
  return {presentFrame->presentReady, std::move(presentFence)};
}

bool SwapchainGeneration::suboptimal() const noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  return impl->suboptimal;
}

SwapchainFrame &SwapchainGeneration::frame(uint32_t imageIndex) noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  return impl->frames[imageIndex];
}

void SwapchainGeneration::release(uint32_t imageIndex) const noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  std::lock_guard lck{impl->mutex};
  vulkan::release_swapchain_image(impl->surface.ctx(), impl->swapchain,
                                  imageIndex);
}

} // namespace strobe::rhi
