#include "io.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/window/window_impl.hpp"

#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>

#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <numbers>
#include <random>
#include <thread>
#include <vector>

#include <strobe/rhi/rhi.hpp>

using namespace strobe;

namespace {

constexpr uint32_t TRIANGLES_PER_BUFFER = 64;
constexpr std::size_t MAX_PUBLISHED_BUFFERS = 64;

struct PublishedTriangles {
  rhi::Buffer buffer;
  rhi::Timepoint ready;
  uint32_t vertexCount;
};

std::vector<vec2> generate_triangles(std::mt19937 &rng) {
  std::uniform_real_distribution<float> centerDistribution{-0.85f, 0.85f};
  std::uniform_real_distribution<float> radiusDistribution{0.01f, 0.10f};
  std::uniform_real_distribution<float> rotationDistribution{
      0.0f, 2.0f * std::numbers::pi_v<float>};

  std::vector<vec2> vertices;
  vertices.reserve(TRIANGLES_PER_BUFFER * 3);

  for (uint32_t triangle = 0; triangle < TRIANGLES_PER_BUFFER; ++triangle) {
    const float centerX = centerDistribution(rng);
    const float centerY = centerDistribution(rng);
    const float radius = radiusDistribution(rng);
    const float rotation = rotationDistribution(rng);

    for (uint32_t vertex = 0; vertex < 3; ++vertex) {
      const float angle =
          rotation +
          static_cast<float>(vertex) *
              (2.0f * std::numbers::pi_v<float> / 3.0f);

      vertices.push_back(vec2{
          centerX + std::cos(angle) * radius,
          centerY + std::sin(angle) * radius,
      });
    }
  }

  return vertices;
}

} // namespace

int main() {
  tracy::SetThreadName("platform");

  Platform::start([] {
    tracy::SetThreadName("render");

    window::WindowImpl window{uvec2{800, 600}, "strobe DMA benchmark"};

    rhi::Device device = rhi::create_device({
        .debug_utils = true,
    });

    rhi::Queue queue = device.get_queue();
    rhi::CommandPool cmdpool = device.create_cmdpool();

    const auto vertexSpv = utility::read_spirv("./vertex.spv");
    const auto fragmentSpv = utility::read_spirv("./fragment.spv");

    rhi::VertexShader vertexShader =
        device.create_vertex_shader({.spirv = vertexSpv});

    rhi::FragmentShader fragmentShader =
        device.create_fragment_shader({.spirv = fragmentSpv});

    rhi::Swapchain swapchain =
        device.create_swapchain({.window = window.ptr()});

    std::mutex publishedMutex;
    std::condition_variable publishedCv;
    std::deque<PublishedTriangles> published;

    std::jthread generator{
        [device, &publishedMutex, &publishedCv,
         &published](std::stop_token stop) mutable {
          tracy::SetThreadName("triangle-generator");

          std::random_device randomDevice;
          std::mt19937 rng{randomDevice()};

          while (!stop.stop_requested()) {
            // Keep the producer from consuming unbounded amounts of memory
            // when rendering/acquire is temporarily stalled.
            {
              std::unique_lock lock{publishedMutex};
              publishedCv.wait(lock, [&] {
                return stop.stop_requested() ||
                       published.size() < MAX_PUBLISHED_BUFFERS;
              });

              if (stop.stop_requested()) {
                break;
              }
            }

            std::vector<vec2> vertices = generate_triangles(rng);

            const uint64_t size =
                static_cast<uint64_t>(vertices.size() * sizeof(vec2));

            rhi::Buffer buffer = device.create_buffer({
                .size = size,
                .bufferUsage =
                    rhi::BufferUsage::transfer_dst |
                    rhi::BufferUsage::vertex,
            });

            // async_upload copies the host data before returning, so the local
            // vertices vector can immediately be destroyed/reused.
            rhi::Timepoint ready =
                device.async_upload(buffer, vertices.data(), size);

            {
              std::lock_guard lock{publishedMutex};

              if (stop.stop_requested()) {
                break;
              }

              published.push_back(PublishedTriangles{
                  .buffer = std::move(buffer),
                  .ready = std::move(ready),
                  .vertexCount =
                      static_cast<uint32_t>(vertices.size()),
              });
            }
          }
        }};

    window.show();

    while (!window.should_close()) {
      window.poll();

      rhi::SwapchainImage frame = swapchain.acquire();

      // Everything drained here belongs exclusively to this frame.
      std::deque<PublishedTriangles> frameTriangles;
      {
        std::lock_guard lock{publishedMutex};
        frameTriangles.swap(published);
      }

      // Allow the producer to refill the bounded publication queue.
      publishedCv.notify_one();

      rhi::CommandBuffer cmd = cmdpool.alloc();
      cmd.begin();

      cmd.transition_image(frame.image(), rhi::ImageLayout::undefined,
                           rhi::ImageLayout::general);

      rhi::Attachment colorAttachment{
          .view = frame.view(),
          .loadOp = rhi::AttachmentLoadOp::clear,
          .storeOp = rhi::AttachmentStoreOp::store,
      };

      cmd.begin_rendering({
          .colorAttachments = &colorAttachment,
      });

      const uvec2 extent = frame.extent();

      cmd.set_viewport({
          .extent = extent,
      });

      cmd.set_scissor({
          .extent = extent,
      });

      rhi::VertexBinding binding{
          .binding = 0,
          .stride = sizeof(vec2),
      };

      rhi::VertexAttribute attribute{
          .location = 0,
          .binding = 0,
          .format = rhi::Format::rg32_float,
          .offset = 0,
      };

      cmd.set_vertex_input(&binding, &attribute);
      cmd.bind_shader(vertexShader);
      cmd.bind_shader(fragmentShader);

      for (const PublishedTriangles &triangles : frameTriangles) {
        // This is a GPU-side wait. It does not block the render thread.
        //
        // All batches use the same DMA timeline, so QueueImpl should coalesce
        // these waits into the greatest required timeline value.
        queue.wait(triangles.ready,
                   rhi::PipelineStage::vertex_attribute_input);

        cmd.bind_vertex_buffer(triangles.buffer);
        cmd.draw(triangles.vertexCount);
      }

      cmd.end_rendering();

      cmd.transition_image(frame.image(), rhi::ImageLayout::general,
                           rhi::ImageLayout::present);

      cmd.end();

      queue.wait(frame);
      queue.submit(&cmd);
      queue.present(std::move(frame));

      // frameTriangles is destroyed here. The buffers were drawn exactly once.
      // CommandBuffer retains them until the graphics submission completes.

      FrameMark;
    }

    generator.request_stop();
    publishedCv.notify_all();
    generator.join();

    // Buffers still waiting in `published` are simply dropped. DMA command
    // buffer retention keeps any in-flight upload resources alive correctly.
  });
}
