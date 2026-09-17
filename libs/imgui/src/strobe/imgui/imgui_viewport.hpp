#pragma once

#include "imgui.h"
#include "strobe/imgui/imgui.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/objects/device.hpp"
#include <GLFW/glfw3.h>

namespace strobe::imgui {

struct Viewport {
public:
  static Viewport *create_viewport(rhi::Device device, GLFWwindow* window) {

    Viewport *viewport = new Viewport({});
    return viewport;
  }

  void render(rhi::CommandBuffer cmd, ImDrawData* drawData) {
       
  }

private:
  explicit Viewport(rhi::Swapchain swapchain) noexcept
      : m_swapchain(swapchain) {}

  rhi::Swapchain m_swapchain;
};

} // namespace strobe::imgui
