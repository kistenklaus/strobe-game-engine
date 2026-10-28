#pragma once

#include "strobe/flow/op_source.hpp"
#include "strobe/rdg/swapchain.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/objects/swapchain.hpp"
#include <memory>
#include <stdexcept>
#include <tracy/Tracy.hpp>

namespace strobe {

inline void rhi_acquire(const flow::ExternalOpContext &ctx) noexcept {
  ZoneScopedN("swapchain::rhi-acquire");
  auto *image = static_cast<rhi::SwapchainImage *>(ctx.arguments[0]);
  auto *swapchain = static_cast<rhi::Swapchain *>(ctx.arguments[1]);
  static_assert(sizeof(rhi::SwapchainImage) == sizeof(void *));
  static_assert(sizeof(rhi::Swapchain) == sizeof(void *));
  std::construct_at(image, swapchain->acquire());
}

inline void rhi_present(const flow::ExternalOpContext &ctx) noexcept {
  ZoneScopedN("swapchain::rhi-present");
  auto *queue = static_cast<rhi::Queue *>(ctx.arguments[0]);
  auto *image = static_cast<rhi::SwapchainImage *>(ctx.arguments[1]);
  FrameMark;
  queue->present(std::move(*image));
  std::destroy_at(image);
}

inline void rhi_wait(const flow::ExternalOpContext &ctx) noexcept {
  ZoneScopedN("swapchain::rhi-wait");
  auto *queue = static_cast<rhi::Queue *>(ctx.arguments[0]);
  auto *image = static_cast<rhi::SwapchainImage *>(ctx.arguments[1]);
  queue->wait(*image);
}

inline void *rhi_image(void *storage) noexcept {
  ZoneScopedN("swapchain::rhi-image");
  auto *swap_img = static_cast<rhi::SwapchainImage *>(storage);
  return const_cast<rhi::Image *>(&swap_img->image());
}

inline void *rhi_view(void *storage) noexcept {
  ZoneScopedN("swapchain::rhi-view");
  auto *swap_img = static_cast<rhi::SwapchainImage *>(storage);
  return const_cast<rhi::ImageView *>(&swap_img->view());
}

static flow::ExternalOpFunction resolve(void *, std::string_view symbol,
                                        uint32_t abi) noexcept {

  if (symbol == "acquire" && abi == 1) {
    return rhi_acquire;
  } else if (symbol == "wait" && abi == 1) {
    return rhi_wait;
  } else if (symbol == "present" && abi == 1) {
    return rhi_present;
  } else {
    fmt::println("Failed to find symbol!");
    std::terminate();
  }
}

static flow::ExternalProjectionFunction
resolve_projection(void *, std::string_view symbol, uint32_t abi) noexcept {
  if (abi == 1 && symbol == "image") {
    return rhi_image;
  }
  if (abi == 1 && symbol == "view") {
    return rhi_view;
  }
  return nullptr;
}

inline flow::Library make_rhi_swapchain_lib() {
  return flow::Library{
      .state = nullptr,
      .resolve = resolve,
      .resolve_projection = resolve_projection,
  };
}

} // namespace strobe
