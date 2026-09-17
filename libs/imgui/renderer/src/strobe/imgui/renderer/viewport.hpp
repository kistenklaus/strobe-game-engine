#pragma once

#include "imgui.h"
#include "strobe/imgui/renderer/frame.hpp"
#include "strobe/imgui/renderer/samplers.hpp"
#include "strobe/imgui/renderer/shaders.hpp"
#include "strobe/rhi/objects/swapchain.hpp"
namespace strobe::imgui::renderer {

struct Viewport {
public:
  static void create_viewport(ImGuiViewport *viewport, rhi::Device device,
                              uint32_t framesInFlight,
                              renderer::Samplers *samplers,
                              renderer::Shaders *shaders,
                              rhi::CommandPool cmdpool) noexcept;
  static void destroy_viewport(ImGuiViewport *viewport) noexcept;
  static void resize(ImGuiViewport *viewport, ImVec2 size) noexcept;
  static void render_viewport(ImGuiViewport *viewport,
                              uint32_t frameIndex) noexcept;

private:
  explicit Viewport(rhi::Device device, rhi::Swapchain swapchain,
                    uint32_t framesInFlight, renderer::Samplers *samplers,
                    renderer::Shaders *shaders, rhi::CommandPool cmdpool) noexcept;
  ~Viewport() noexcept;

  rhi::Device m_device;
  rhi::Queue m_queue;
  rhi::Swapchain m_swapchain;
  uint32_t m_framesInFlight;
  renderer::Frame *m_frames;
  renderer::Samplers *m_samplers;
  renderer::Shaders *m_shaders;
  rhi::CommandPool m_cmdpool;
};

void cleanup_viewports() noexcept;

} // namespace strobe::imgui::renderer
