#pragma once

#include "strobe/rhi/types/image_usage.hpp"
#include <GLFW/glfw3.h>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Swapchain creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} swapchain_info.hpp SwapchainInfo
 *
 * Describes the window, presentation mode, and image usage of a swapchain.
 */
// [SwapchainInfo]
struct SwapchainInfo {
  GLFWwindow *window = nullptr;
  bool vsync = false;
  ImageUsage imageUsage = ImageUsage::color_attachment;
};
// [SwapchainInfo]

} // namespace strobe::rhi
