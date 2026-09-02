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

#include <cassert>
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

constexpr uint32_t TRIANGLES_PER_BUFFER = 8196;

// Bounds publication latency and the number of individual Vulkan buffers.
constexpr std::size_t MAX_PUBLISHED_BUFFERS = 64;

// Also bound memory if buffers become variable-sized later.
constexpr uint64_t MAX_PUBLISHED_BYTES = 16ull * 1024ull * 1024ull;

struct PublishedTriangles {
  rhi::Buffer buffer;
  rhi::Timepoint ready;
  uint32_t vertexCount;
  uint64_t bytes;
};

struct PublicationQueue {
  std::mutex mutex;
  std::condition_variable cv;
  std::deque<PublishedTriangles> buffers;
  uint64_t bytes = 0;
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
          rotation + static_cast<float>(vertex) *
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

    window::WindowImpl window{
        uvec2{800, 600},
        "strobe DMA benchmark",
    };

    rhi::Device device = rhi::create_device({
        .debug_utils = false,
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
        device.create_swapchain({.window = window.ptr(), .vsync = false});

    PublicationQueue publication;

    std::jthread generator{
        [device, &publication](std::stop_token stop) mutable {
          tracy::SetThreadName("triangle-generator");

          std::random_device randomDevice;
          std::mt19937 rng{randomDevice()};

          while (!stop.stop_requested()) {
            std::vector<vec2> vertices = generate_triangles(rng);

            const uint64_t size =
                static_cast<uint64_t>(vertices.size() * sizeof(vec2));

            assert(size <= MAX_PUBLISHED_BYTES);

            /*
             * There is one producer, so checking capacity before creating the
             * buffer is sufficient. With multiple producers, capacity would
             * need to be reserved while holding the mutex.
             */
            {
              std::unique_lock lock{publication.mutex};

              publication.cv.wait(lock, [&] {
                return stop.stop_requested() ||
                       (publication.buffers.size() < MAX_PUBLISHED_BUFFERS &&
                        publication.bytes + size <= MAX_PUBLISHED_BYTES);
              });

              if (stop.stop_requested()) {
                break;
              }
            }

            rhi::Buffer buffer = device.create_buffer({
                .size = size,
                .bufferUsage =
                    rhi::BufferUsage::transfer_dst | rhi::BufferUsage::vertex,
            });

            /*
             * async_upload copies the host data before returning. The local
             * vertices vector can therefore be reused immediately.
             */
            rhi::Timepoint ready =
                device.async_upload(buffer, vertices.data(), size);

            {
              std::lock_guard lock{publication.mutex};

              if (stop.stop_requested()) {
                break;
              }

              publication.bytes += size;

              publication.buffers.push_back(PublishedTriangles{
                  .buffer = std::move(buffer),
                  .ready = std::move(ready),
                  .vertexCount = static_cast<uint32_t>(vertices.size()),
                  .bytes = size,
              });
            }
          }
        }};

    window.show();

    while (true) {
      window.poll();
      if (window.should_close()) {
        break;
      }

      rhi::SwapchainImage frame = swapchain.acquire();

      /*
       * Move only completed DMA uploads into this frame. All uploads use the
       * same monotonically increasing DMA timeline, so if the front is not
       * complete, no later entry can be complete either.
       *
       * relaxed_poll() may give a false negative when the GC cache is slightly
       * behind. That only delays the triangles by one frame.
       */
      std::deque<PublishedTriangles> frameTriangles;
      bool releasedCapacity = false;

      {
        std::lock_guard lock{publication.mutex};

        while (!publication.buffers.empty()) {
          PublishedTriangles &front = publication.buffers.front();

          if (!front.ready.relaxed_poll()) {
            break;
          }

          assert(publication.bytes >= front.bytes);
          publication.bytes -= front.bytes;

          frameTriangles.push_back(std::move(front));
          publication.buffers.pop_front();
          releasedCapacity = true;
        }
      }

      if (releasedCapacity) {
        publication.cv.notify_one();
      }

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
        /*
         * The upload is known to be completed, so this records the GPU memory
         * dependency without forcing DMA to commit anything.
         */
        queue.wait(triangles.ready, rhi::PipelineStage::vertex_attribute_input);

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

      /*
       * frameTriangles is destroyed here. The command buffer retains the
       * buffers until the graphics submission completes.
       */
      FrameMark;
    }

    generator.request_stop();
    publication.cv.notify_all();
    generator.join();

    /*
     * Anything still queued is dropped. DMA command-buffer retention keeps
     * resources referenced by an in-flight upload alive.
     */
  });
}
