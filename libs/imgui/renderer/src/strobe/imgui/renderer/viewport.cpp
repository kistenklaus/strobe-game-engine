#include "strobe/imgui/renderer/viewport.hpp"
#include "imgui.h"
#include "strobe/imgui/renderer/draw.hpp"
#include "strobe/rhi/types/image_usage.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include <fmt/ostream.h>

namespace strobe::imgui::renderer {

void Viewport::create_viewport(ImGuiViewport *viewport, rhi::Device device,
                               uint32_t framesInFlight,
                               renderer::Samplers *samplers,
                               renderer::Shaders *shaders,
                               rhi::CommandPool cmdpool) noexcept {
  assert(viewport);
  assert(viewport->RendererUserData == nullptr);
  GLFWwindow *window = static_cast<GLFWwindow *>(viewport->PlatformHandle);
  assert(window);

  auto swapchain = device.create_swapchain({
      .window = window,
      .vsync = false,
      .imageUsage = rhi::ImageUsage::color_attachment,
  });
  viewport->RendererUserData =
      new Viewport(std::move(device), std::move(swapchain), framesInFlight,
                   samplers, shaders, std::move(cmdpool));
}

void Viewport::destroy_viewport(ImGuiViewport *viewport) noexcept {
  assert(viewport);
  auto *backend_viewport = static_cast<Viewport *>(viewport->RendererUserData);
  delete backend_viewport;
  viewport->RendererUserData = nullptr;
}

void Viewport::resize(ImGuiViewport *viewport, ImVec2 size) noexcept {
  assert(viewport);
  auto *backend_viewport = static_cast<Viewport *>(viewport->RendererUserData);
  assert(backend_viewport);
  backend_viewport->m_swapchain.resize_hint(
      uvec2{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)});
}

void Viewport::render_viewport(ImGuiViewport *viewport,
                               uint32_t frameIndex) noexcept {
  assert(viewport);
  auto *backend_viewport = static_cast<Viewport *>(viewport->RendererUserData);
  assert(backend_viewport);
  auto &frame = backend_viewport->m_frames[frameIndex];
  frame.ready.wait();
  auto &queue = backend_viewport->m_queue;

  auto swapchainImage = backend_viewport->m_swapchain.acquire();
  rhi::CommandBuffer cmd = backend_viewport->m_cmdpool.alloc();
  cmd.begin();
  cmd.transition_image(swapchainImage.image(), rhi::ImageLayout::undefined,
                       rhi::ImageLayout::attachment);

  renderer::draw(cmd, swapchainImage.view(), rhi::AttachmentLoadOp::clear,
                 viewport->DrawData, backend_viewport->m_device, frame,
                 backend_viewport->m_samplers, backend_viewport->m_shaders);

  cmd.transition_image(swapchainImage.image(), rhi::ImageLayout::attachment,
                       rhi::ImageLayout::present);

  cmd.end();
  queue.wait(swapchainImage);
  frame.ready = queue.submit(&cmd);
  queue.present(std::move(swapchainImage));
}

Viewport::Viewport(rhi::Device device, rhi::Swapchain swapchain,
                   uint32_t framesInFlight, renderer::Samplers *samplers,
                   renderer::Shaders *shaders,
                   rhi::CommandPool cmdpool) noexcept
    : m_device(std::move(device)),
      m_queue(m_device.get_queue(rhi::QueueFlags::present |
                                 rhi::QueueFlags::transfer |
                                 rhi::QueueFlags::graphics)),
      m_swapchain(std::move(swapchain)), m_framesInFlight(framesInFlight),
      m_frames(new Frame[framesInFlight]), m_samplers(samplers),
      m_shaders(shaders), m_cmdpool(std::move(cmdpool)) {}

Viewport::~Viewport() noexcept { delete[] m_frames; }

void cleanup_viewports() noexcept {
  auto &platform_io = ImGui::GetPlatformIO();
  for (ImGuiViewport *viewport : platform_io.Viewports) {
    if (viewport->RendererUserData) {
      Viewport::destroy_viewport(viewport);
    }
  }
}

} // namespace strobe::imgui::renderer
