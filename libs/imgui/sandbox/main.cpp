
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "strobe/imgui/imgui.hpp"
#include "strobe/platform/platform.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/rhi.hpp"
#include "strobe/rhi/types/attachment_load_op.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/window/window_impl.hpp"
#include <fmt/printf.h>

using namespace strobe;

int main() {
  Platform::start([]() {
    window::WindowImpl window{uvec2(800, 600), "FLOATING"};
    window.resizable(true);

    rhi::Device device = rhi::create_device({
        .debug_utils = true,
    });

    rhi::Queue queue = device.get_queue();
    rhi::Swapchain swapchain =
        device.create_swapchain({.window = window.ptr(), .vsync = false});

    rhi::CommandPool cmdpool = device.create_cmdpool();

    strobe::imgui::init(window.ptr(), device);

    window.visible(true);

    while (!window.should_close()) {
      rhi::SwapchainImage frame = swapchain.acquire();

      imgui::begin_frame();
      ImGui::ShowDemoWindow();

      rhi::CommandBuffer cmd = cmdpool.alloc();
      cmd.begin();
      imgui::render(cmd, {.target = frame,
                          .inital_layout = rhi::ImageLayout::undefined,
                          .final_layout = rhi::ImageLayout::present});
      cmd.end();

      queue.wait(frame);
      rhi::Timepoint done = queue.submit(&cmd);
      queue.present(frame);
      done.wait();

      window.poll();
    }
    strobe::imgui::shutdown();
  });
}
