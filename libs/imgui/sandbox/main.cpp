
#include "imgui.h"
#include "strobe/core/containers/vector.hpp"
#include "strobe/imgui/platform.hpp"
#include "strobe/imgui/renderer.hpp"
#include "strobe/platform.hpp"
#include "strobe/platform/window.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/rhi.hpp"
#include <fmt/printf.h>

using namespace strobe;

int main() {
  platform::Window window{uvec2(800, 600), "FLOATING"};
  window.resizable(true);

  rhi::Device device = rhi::create_device({
      .debug_utils = true,
  });

  rhi::Queue queue = device.get_queue();
  rhi::Swapchain swapchain =
      device.create_swapchain({.window = window.window_ptr(), .vsync = false});

  rhi::CommandPool cmdpool = device.create_cmdpool();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  {
    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImFont *regular = io.Fonts->AddFontFromFileTTF(
        "./third_party/fonts/jetbrains/JetBrainsMonoNL-Light.ttf", 16.0f);
    assert(regular);
    io.FontDefault = regular;
  }

  {
    uint32_t framesInFlight = 3;
    imgui::Platform platform{&window};
    imgui::Renderer renderer{device, framesInFlight};

    window.visible(true);

    struct Frame {
      rhi::Timepoint ready{};
    };
    Vector<Frame> frames{framesInFlight};
    uint32_t frameIndex = 0;

    while (!window.should_close()) {
      rhi::SwapchainImage image = swapchain.acquire();

      auto &frame = frames[frameIndex];
      frame.ready.wait();

      {
        ZoneScopedN("Platform::new_frame");
        platform.new_frame();
      }

      ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                   ImGuiDockNodeFlags_None);

      {
        ZoneScopedN("ShowDemoWindow");
        ImGui::ShowDemoWindow();
      }
      {
        ZoneScopedN("ImGui::Render");
        ImGui::Render();
      }

      auto *drawData = ImGui::GetDrawData();




      rhi::CommandBuffer cmd = cmdpool.alloc();
      cmd.begin();

      ImGui::UpdatePlatformWindows();

      renderer.render(cmd, drawData, image.view());

      cmd.end();

      queue.wait(image);
      frame.ready = queue.submit(&cmd);

      queue.present(image);

      frameIndex = (frameIndex + 1) % framesInFlight;
      FrameMark;
    }
  }

  ImGui::DestroyContext();
}
