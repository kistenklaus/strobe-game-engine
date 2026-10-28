#include "rhi_swapchain_lib.hpp"
#include "strobe/core/sync/allocator.hpp"
#include "strobe/flow/channel.hpp"
#include "strobe/flow/compiler_plugins.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/loop.hpp"
#include "strobe/platform/window.hpp"
#include "strobe/rdg/buffer.hpp"
#include "strobe/rdg/command_buffer.hpp"
#include "strobe/rdg/command_pool.hpp"
#include "strobe/rdg/plugins.hpp"
#include "strobe/rdg/rdg.hpp"
#include "strobe/rdg/swapchain.hpp"
#include "strobe/rhi/rhi.hpp"

struct Frame {
  int id;
  strobe::vec4 color;
};

int main() {
  using namespace strobe;

  flow::CompilerPlugins plugins;
  rdg::append_plugins(plugins);
  flow::begin(plugins);
  {
    rdg::begin();

    rdg::Swapchain swapchain{"main"};

    rdg::CommandPool cmdpool{};

    flow::u64 size = 64;

    flow::loop(
        [&] {
          auto frame = flow::recv<Frame>("fchan");
          auto target = rdg::acquire(swapchain);

          rdg::CommandBuffer cmd1 = cmdpool.alloc();

          // somehow doesn't get moved outside of the loop!
          rdg::Buffer buffer{{
              .size = size,
              .bufferUsage = rhi::BufferUsage::storage,
          }};

          flow::vec4 color = flow::project<strobe::vec4>(
              frame, "color", flow::ProjectionKind::disjoint,
              [](Frame &frame) noexcept {
                ZoneScopedN("project-frame-color");
                return &frame.color;
              });

          rdg::Attachment attachment{
              .view = target.view(),
              .loadOp = rhi::AttachmentLoadOp::clear,
              .clearValue = rdg::ClearValue::float4(color),
          };

          cmd1.begin_rendering({
              .colorAttachments = &attachment,
          });

          cmd1.end_rendering();
          cmd1.present(target);

          auto timepoint = rdg::submit(cmd1);
        },
        4);
    rdg::end();
  }

  auto prog = flow::end();

  rhi::Device device = rhi::create_device({
      .debug_utils = true,
  });
  prog.bind_external("device", &device);

  // swapchain runtime!
  prog.bind_library("swapchain", make_rhi_swapchain_lib());

  platform::Window window{{
      .size = uvec2{600, 600},
      .title = "abc",
  }};

  window.visible(true);
  rhi::Swapchain swapchain =
      device.create_swapchain({.window = window.window_ptr()});
  rdg::swapchain_runtime runtime;
  std::memcpy(&runtime, &swapchain, sizeof(void *));
  prog.bind_external("main", &runtime);

  flow::Channel<Frame> tx{&prog, "fchan"};

  sync::allocator alloc;
  sync::JobScheduler scheduler{&alloc, 2};

  while (!window.should_close()) {
    tx.send(Frame{.id = 0, .color = {1.0f, 0.0f, 0.0f, 1.0f}});
    prog.submit(&scheduler);
    prog.rethrow_execution_error();
  }
}
