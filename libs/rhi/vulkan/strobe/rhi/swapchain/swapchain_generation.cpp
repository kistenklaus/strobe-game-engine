#include "strobe/rhi/swapchain/swapchain_generation.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/swapchain/swapchain_generation_impl.hpp"
#include "strobe/rhi/swapchain/swapchain_image_impl.hpp"
#include "strobe/rhi/sync/binary_semaphore_node.hpp"
#include "strobe/rhi/vulkan/swapchain.hpp"
#include <exception>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace strobe::rhi {

SwapchainGeneration::SwapchainGeneration(const SwapchainGeneration &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<SwapchainGenerationImpl>(m_handle);
  }
}
SwapchainGeneration::SwapchainGeneration(SwapchainGeneration &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

SwapchainGeneration &
SwapchainGeneration::operator=(const SwapchainGeneration &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<SwapchainGenerationImpl>(o.m_handle);
  }
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

SwapchainGeneration &
SwapchainGeneration::operator=(SwapchainGeneration &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

SwapchainGeneration::~SwapchainGeneration() noexcept {
  unpin_void_handle<SwapchainGenerationImpl>(m_handle);
}

SwapchainImage SwapchainGeneration::acquire() {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  vulkan::Context *ctx = impl->surface.ctx();

  BinarySemaphore imageAvailable = impl->semPool.allocate();

  uint32_t imageIndex;
  const auto result = vulkan::acquire_next_swapchain_image(
      ctx, impl->swapchain,
      {
          .timeout = std::numeric_limits<uint64_t>::max(),
          .signalSemaphore = imageAvailable.signal(),
      },
      &imageIndex);
  switch (result) {
  case vulkan::SwapchainAcquireStatus::success:
    impl->frames[imageIndex].imageAvailable = std::move(imageAvailable);
    return SwapchainImage{make_void_handle<SwapchainImageImpl>(
        impl->get_swapchain_image_handle_allocator(), //
        *this, imageIndex)};
  case vulkan::SwapchainAcquireStatus::suboptimal:
    impl->frames[imageIndex].imageAvailable = std::move(imageAvailable);
    impl->suboptimal = true;
    return SwapchainImage{make_void_handle<SwapchainImageImpl>(
        impl->get_swapchain_image_handle_allocator(), //
        *this, imageIndex)};
  case vulkan::SwapchainAcquireStatus::out_of_date:
    return {};
  case vulkan::SwapchainAcquireStatus::timeout:
  case vulkan::SwapchainAcquireStatus::not_ready:
    throw std::runtime_error("unexpected swapchain acquire result");
  }
  std::unreachable();
}

std::pair<BinarySemaphore, Fence> SwapchainGeneration::present() {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  auto *presentFrame = static_cast<SwapchainPresentFrame *>(
      impl->get_present_frame_allocator().allocate(
          sizeof(SwapchainPresentFrame), alignof(SwapchainPresentFrame)));
  std::construct_at(presentFrame, *this, impl->semPool.allocate());

  Fence presentFence = impl->fencePool.allocate(presentFrame, [](void *ptr) {
    auto *presentFrame = static_cast<SwapchainPresentFrame *>(ptr);
    // the generations own the backing allocator so we have to be careful here.
    SwapchainGeneration generation = std::move(presentFrame->generation);
    std::destroy_at(presentFrame);
    object_handle_ptr<SwapchainGenerationImpl>(generation)
        ->get_present_frame_allocator()
        .deallocate(presentFrame);
  });
  return {std::move(presentFrame->presentReady), std::move(presentFence)};
}

bool SwapchainGeneration::suboptimal() const noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  return impl->suboptimal;
}

SwapchainFrame &SwapchainGeneration::frame(uint32_t imageIndex) noexcept {
  auto *impl = void_handle_ptr<SwapchainGenerationImpl>(m_handle);
  return impl->frames[imageIndex];
}

} // namespace strobe::rhi
