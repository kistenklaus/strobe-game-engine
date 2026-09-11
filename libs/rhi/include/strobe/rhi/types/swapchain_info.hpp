#pragma once

#include "strobe/rhi/types/image_usage.hpp"
#include <GLFW/glfw3.h>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Swapchain creation information.
 *
 * Describes the window, presentation mode, and image usage of a swapchain.
 */
struct SwapchainInfo {
  /** GLFW window used for presentation. */
  GLFWwindow *window = nullptr;

  /** Whether presentation is synchronized to the display refresh. */
  bool vsync = false;

  /** Intended usage of swapchain images. */
  ImageUsage imageUsage = ImageUsage::color_attachment;
};

} // namespace strobe::rhi
