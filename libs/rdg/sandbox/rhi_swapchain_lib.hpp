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
  auto *image = static_cast<rhi::SwapchainImage *>(ctx.arguments[0]);
  auto *swapchain = static_cast<rhi::Swapchain *>(ctx.arguments[1]);
  static_assert(sizeof(rhi::SwapchainImage) == sizeof(void *));
  static_assert(sizeof(rhi::Swapchain) == sizeof(void *));
  std::construct_at(image, swapchain->acquire());
}

inline void rhi_present(const flow::ExternalOpContext &ctx) noexcept {
  auto *queue = static_cast<rhi::Queue *>(ctx.arguments[0]);
  auto *image = static_cast<rhi::SwapchainImage *>(ctx.arguments[1]);
  FrameMark;
  queue->present(std::move(*image));
  std::destroy_at(image);
}

inline void rhi_wait(const flow::ExternalOpContext &ctx) noexcept {
  auto *queue = static_cast<rhi::Queue *>(ctx.arguments[0]);
  auto *image = static_cast<rhi::SwapchainImage *>(ctx.arguments[1]);
  queue->wait(*image);
}

inline void rhi_get_image(const flow::ExternalOpContext &ctx) noexcept {
  auto *img = static_cast<rhi::Image *>(ctx.arguments[0]);
  auto *swap_img = static_cast<rhi::SwapchainImage *>(ctx.arguments[1]);
  std::construct_at(img, swap_img->image());
}

inline void rhi_get_view(const flow::ExternalOpContext &ctx) noexcept {
  auto *view = static_cast<rhi::ImageView *>(ctx.arguments[0]);
  auto *swap_img = static_cast<rhi::SwapchainImage *>(ctx.arguments[1]);
  std::construct_at(view, swap_img->view());
}

static flow::ExternalOpFunction resolve(void *, std::string_view symbol,
                                        uint32_t abi) noexcept {

  if (symbol == "acquire" && abi == 1) {
    return rhi_acquire;
  } else if (symbol == "wait" && abi == 1) {
    return rhi_wait;
  } else if (symbol == "present" && abi == 1) {
    return rhi_present;
  } else if (symbol == "get_image" && abi == 1) {
    return rhi_get_image;
  }  else if (symbol == "get_view" && abi == 1) {
    return rhi_get_view;
  } else {
    fmt::println("Failed to find symbol!");
    std::terminate();
  }
}

inline flow::Library make_rhi_swapchain_lib() {
  return flow::Library{
      .state = nullptr,
      .resolve = resolve,
  };
}

} // namespace strobe
