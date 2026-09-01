
#include "io.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/types/access_scope.hpp"
#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/window/window_impl.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <common/TracySystem.hpp>
#include <thread>
#include <tracy/Tracy.hpp>
#include <unistd.h>
#include <vulkan/vulkan_core.h>

#include <strobe/rhi/rhi.hpp>

using namespace strobe;
using namespace std::chrono;
using namespace std::chrono_literals;

int main() {
  // #ifdef STROBE_TRACY
  //   fmt::println("waiting for tracy");
  //   while (!TracyIsConnected) {
  //     std::this_thread::yield();
  //   }
  //   fmt::println("tracy connected");
  // #endif
  tracy::SetThreadName("platform");
  Platform::start([]() {
    tracy::SetThreadName("main");
    window::WindowImpl window{uvec2{800, 600}, "strobe"};

    rhi::Device device = strobe::rhi::create_device({
        .debug_utils = false,
    });
    rhi::Queue queue = device.get_queue();
    rhi::CommandPool cmdpool = device.create_cmdpool();

    const auto vertexSpv = utility::read_spirv("./vertex.spv");
    const auto fragmentSpv = utility::read_spirv("./fragment.spv");
    rhi::VertexShader vertex =
        device.create_vertex_shader({.spirv = vertexSpv});
    rhi::FragmentShader fragment =
        device.create_fragment_shader({.spirv = fragmentSpv});

    rhi::Swapchain swapchain =
        device.create_swapchain({.window = window.ptr()});

    rhi::Buffer vertex1 = device.create_buffer({
        .size = 24,
        .bufferUsage =
            rhi::BufferUsage::transfer_dst | rhi::BufferUsage::vertex,
    });

    rhi::Buffer vertex2 = device.create_buffer({
        .size = 24,
        .bufferUsage =
            rhi::BufferUsage::transfer_dst | rhi::BufferUsage::vertex,
    });

    window.show();

    while (!window.should_close()) {
      window.poll();
      rhi::SwapchainImage frame = swapchain.acquire();
      fmt::println("post-acquire");

      // std::this_thread::sleep_for(10ms);
      rhi::CommandBuffer cmd = cmdpool.alloc();
      cmd.begin();

      cmd.transition_image(frame.image(), rhi::ImageLayout::undefined,
                           rhi::ImageLayout::general);

      vec2 v1[3] = {
          {-0.65f, 0.0f}, //
          {0.65f, 0.0f},  //
          {0.0f, 0.75f},  //
      };
      cmd.update(vertex1, v1, sizeof(v1));
      cmd.memory_barrier(rhi::access::transfer_write,
                         rhi::access::vertex_attribute_read);

      rhi::Attachment colorAttachment0{
          .view = frame.view(),
          .loadOp = rhi::AttachmentLoadOp::clear,
          .storeOp = rhi::AttachmentStoreOp::store,
      };
      cmd.begin_rendering({.colorAttachments = &colorAttachment0});
      { // setup rendering state
        const uvec2 swapchainExtent = frame.extent();
        cmd.set_viewport({
            .extent = swapchainExtent,
        });
        cmd.set_scissor({
            .extent = swapchainExtent,
        });
        {
          rhi::VertexBinding binding{
              .binding = 0,
              .stride = sizeof(float) * 2,
          };
          rhi::VertexAttribute attrib{
              .location = 0,
              .binding = 0,
              .format = rhi::Format::rg32_float,
              .offset = 0,
          };
          cmd.set_vertex_input(&binding, &attrib);
        }
        cmd.bind_shader(vertex);
        cmd.bind_shader(fragment);
      }
      cmd.bind_vertex_buffer(vertex1);
      // for (uint32_t i = 0; i < 10000; ++i) {
      cmd.draw(3);
      // }
      cmd.end_rendering();

      // std::this_thread::sleep_for(10us);

      cmd.transition_image(frame.image(), rhi::ImageLayout::general,
                           rhi::ImageLayout::present);
      cmd.end();

      queue.wait(frame);
      fmt::println("waited");
      queue.submit(&cmd);
      fmt::println("submitted");
      queue.present(frame);
      fmt::println("presented");
      fmt::println("{}", std::chrono::high_resolution_clock::now().time_since_epoch().count());

      FrameMark;
    }
  });
}
