#pragma once

#include "strobe/rhi/types/image_usage.hpp"
#include <GLFW/glfw3.h>

namespace strobe::rhi {

struct SwapchainInfo {
  GLFWwindow* window = nullptr;
  bool vsync = false;
  ImageUsage imageUsage = ImageUsage::color_attachment;
};

}
