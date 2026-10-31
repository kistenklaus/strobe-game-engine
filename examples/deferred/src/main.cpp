

#include "deferred.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/swapchain_image.hpp"
#include "strobe/rhi/rhi.hpp"
#include "window.hpp"
#include <fmt/ostream.h>
#include <tracy/Tracy.hpp>
#include "strobe/core/color/color.hpp"

int main() {
  Window window{{
      .size = strobe::uvec2{800, 600},
      .title = "FLOATING",
  }};

  strobe::rhi::Device device = strobe::rhi::create_device({
      .debug_utils = true,
  });

  strobe::rhi::Queue queue = device.get_queue();
  strobe::rhi::CommandPool cmdpool = device.create_cmdpool();

  strobe::rhi::Swapchain swapchain = device.create_swapchain({
      .window = window.native(),
      .vsync = false,
      .imageUsage = strobe::rhi::ImageUsage::storage,
      .srgb = false,
  });

  window.show();

  std::array<strobe::rhi::Timepoint, 2> fences;

  DeferredRenderer deferred{device, fences.size()};

  uint32_t frameIndex = 0;
  while (!window.should_close()) {

    fences[frameIndex].wait();

    strobe::rhi::SwapchainImage target = swapchain.acquire();

    strobe::rhi::CommandBuffer cmd = cmdpool.alloc();
    cmd.begin();
    deferred.render(
        cmd,
        {
            .color = target.image(),
            .colorStorageDescriptor = target.storage_image_descriptor(),
        },
        frameIndex);
    cmd.end();

    queue.wait(target);
    fences[frameIndex] = queue.submit(&cmd);
    queue.present(std::move(target));

    frameIndex = frameIndex + 1;
    if (frameIndex == fences.size()) {
      frameIndex = 0;
    }
    FrameMark;
  }
}
