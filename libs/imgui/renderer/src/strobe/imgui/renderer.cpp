#include "strobe/imgui/renderer.hpp"
#include "imgui.h"
#include "strobe/imgui/renderer/draw.hpp"
#include "strobe/imgui/renderer/frame.hpp"
#include "strobe/imgui/renderer/samplers.hpp"
#include "strobe/imgui/renderer/shaders.hpp"
#include "strobe/imgui/renderer/texture.hpp"
#include "strobe/imgui/renderer/viewport.hpp"

namespace strobe::imgui {

static void renderer_create_window(ImGuiViewport *viewport);

static void renderer_destroy_window(ImGuiViewport *viewport) {
  renderer::Viewport::destroy_viewport(viewport);
}

static void renderer_set_window_size(ImGuiViewport *viewport, ImVec2 size) {
  renderer::Viewport::resize(viewport, size);
}

static void renderer_render_window(ImGuiViewport *viewport, void *arg);

struct RendererImpl {
  rhi::Device device;
  uint32_t framesInFlight;
  renderer::Frame *frames;
  renderer::Samplers samplers;
  renderer::Shaders shaders;
  uint32_t frameIndex;
  rhi::CommandPool cmdpool;

  RendererImpl(rhi::Device device, uint32_t framesInFlight) noexcept
      : device(std::move(device)), framesInFlight(framesInFlight),
        frames{new renderer::Frame[framesInFlight]}, samplers(this->device),
        shaders(this->device), frameIndex(0),
        cmdpool(this->device.create_cmdpool()) {
    auto &io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    io.BackendRendererUserData = this;
    {
      auto &platform_io = ImGui::GetPlatformIO();
      platform_io.DrawCallback_ResetRenderState = [](const ImDrawList *,
                                                     const ImDrawCmd *) {};
      platform_io.DrawCallback_SetSamplerLinear = [](const ImDrawList *,
                                                     const ImDrawCmd *) {};
      platform_io.DrawCallback_SetSamplerNearest = [](const ImDrawList *,
                                                      const ImDrawCmd *) {};
      assert(platform_io.DrawCallback_ResetRenderState !=
             platform_io.DrawCallback_SetSamplerLinear);
      assert(platform_io.DrawCallback_ResetRenderState !=
             platform_io.DrawCallback_SetSamplerNearest);
      assert(platform_io.DrawCallback_SetSamplerLinear !=
             platform_io.DrawCallback_SetSamplerNearest);

      platform_io.Renderer_CreateWindow = renderer_create_window;
      platform_io.Renderer_DestroyWindow = renderer_destroy_window;
      platform_io.Renderer_SetWindowSize = renderer_set_window_size;
      platform_io.Renderer_RenderWindow = renderer_render_window;
      platform_io.Renderer_SwapBuffers = nullptr;
    }
  }

  ~RendererImpl() noexcept {
    renderer::cleanup_viewports();
    renderer::cleanup_texture();
    {
      auto &io = ImGui::GetIO();
      io.BackendRendererUserData = nullptr;
      io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
      io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
      io.BackendFlags &= ~ImGuiBackendFlags_RendererHasViewports;
    }

    {
      auto &platform_io = ImGui::GetPlatformIO();
      platform_io.DrawCallback_ResetRenderState = nullptr;
      platform_io.DrawCallback_SetSamplerLinear = nullptr;
      platform_io.DrawCallback_SetSamplerNearest = nullptr;
      platform_io.Renderer_CreateWindow = nullptr;
      platform_io.Renderer_DestroyWindow = nullptr;
      platform_io.Renderer_SetWindowSize = nullptr;
      platform_io.Renderer_RenderWindow = nullptr;
      platform_io.Renderer_SwapBuffers = nullptr;
    }

    delete[] frames;
  }
};

static void renderer_render_window(ImGuiViewport *viewport, void *) {
  auto *impl =
      static_cast<RendererImpl *>(ImGui::GetIO().BackendRendererUserData);
  assert(impl);
  renderer::Viewport::render_viewport(viewport, impl->frameIndex);
}

static void renderer_create_window(ImGuiViewport *viewport) {
  auto *impl =
      static_cast<RendererImpl *>(ImGui::GetIO().BackendRendererUserData);
  assert(impl);
  renderer::Viewport::create_viewport(viewport, impl->device,
                                      impl->framesInFlight, &impl->samplers,
                                      &impl->shaders, impl->cmdpool);
}

Renderer::Renderer(rhi::Device device, uint32_t framesInFlight) noexcept
    : m_internals(new RendererImpl(std::move(device), framesInFlight)) {}

Renderer::Renderer(Renderer &&o) noexcept
    : m_internals(std::exchange(o.m_internals, nullptr)) {}

Renderer &Renderer::operator=(Renderer &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (m_internals) {
    delete static_cast<RendererImpl *>(m_internals);
  }
  m_internals = std::exchange(o.m_internals, nullptr);
  return *this;
}

Renderer::~Renderer() noexcept {
  if (m_internals) {
    delete static_cast<RendererImpl *>(m_internals);
  }
}

void Renderer::render(rhi::CommandBuffer cmd, const ImDrawData *drawData,
                      rhi::ImageView target, rhi::ImageLayout initialLayout,
                      rhi::ImageLayout finalLayout) noexcept {
  assert(drawData);
  assert(drawData->Valid);

  auto *impl = static_cast<RendererImpl *>(m_internals);
  assert(impl);

  const uvec2 drawExtent{
      uint32_t(drawData->DisplaySize.x * drawData->FramebufferScale.x + 0.5f),
      uint32_t(drawData->DisplaySize.y * drawData->FramebufferScale.y + 0.5f),
  };

  auto &frame = impl->frames[impl->frameIndex];

  renderer::update_textures(impl->device, cmd, drawData);

  if (initialLayout != rhi::ImageLayout::attachment) {
    cmd.transition_image(target.image(), initialLayout,
                         rhi::ImageLayout::attachment);
  }

  renderer::draw(cmd, target,
                 initialLayout == rhi::ImageLayout::undefined
                     ? rhi::AttachmentLoadOp::clear
                     : rhi::AttachmentLoadOp::load,
                 drawData, impl->device, frame, &impl->samplers,
                 &impl->shaders);

  if (finalLayout != rhi::ImageLayout::attachment) {
    cmd.transition_image(target.image(), rhi::ImageLayout::attachment,
                         finalLayout);
  }

  ImGui::RenderPlatformWindowsDefault();

  impl->frameIndex = (impl->frameIndex + 1) % impl->framesInFlight;
}

} // namespace strobe::imgui
