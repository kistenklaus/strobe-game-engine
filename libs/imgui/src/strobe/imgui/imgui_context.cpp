#include "strobe/imgui/imgui_context.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "strobe/imgui/imgui.hpp"

namespace strobe::imgui {

Context *g_context = nullptr;

void init(GLFWwindow *window, rhi::Device device,
          uint32_t framesInFlight) noexcept {
  IMGUI_CHECKVERSION();
  assert(g_context == nullptr);
  g_context = new Context(std::move(device), framesInFlight);
  ImGui::CreateContext();

  {
    auto &io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImFont *regular = io.Fonts->AddFontFromFileTTF(
        "./third_party/fonts/jetbrains/JetBrainsMonoNL-Light.ttf", 16.0f);
    assert(regular);
    io.FontDefault = regular;
  }

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
  }

  ImGui_ImplGlfw_InitForVulkan(window, true);
}

void shutdown() noexcept {
  auto &platform_io = ImGui::GetPlatformIO();
  for (ImTextureData *texture : platform_io.Textures) {
    if (texture->BackendUserData) {
      // destroy_texture(texture);
    }
  }
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  delete g_context;
  g_context = nullptr;
}

} // namespace strobe::imgui
