#pragma once

#include "strobe/rhi/objects/device.hpp"

namespace strobe::imgui {

void init(GLFWwindow *window, rhi::Device device, uint32_t framesInFlight) noexcept;
void shutdown() noexcept;

void begin_frame() noexcept;

struct RenderInfo {
  rhi::SwapchainImage target;
  rhi::ImageLayout inital_layout;
  rhi::ImageLayout final_layout;
};

void render(rhi::CommandBuffer cmd, RenderInfo info) noexcept;

} // namespace strobe::imgui
