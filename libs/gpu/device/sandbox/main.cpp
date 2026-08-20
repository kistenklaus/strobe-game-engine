
#include "io.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/gpu/device/attachment_load_op.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/fragment_shader.hpp"
#include "strobe/gpu/device/memory_pool.hpp"
#include "strobe/gpu/device/queue_flags.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"
#include "strobe/window/window_impl.hpp"
#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan_core.h>

using namespace strobe;
using namespace strobe::window;
using namespace strobe::gpu;

enum class ResourceMix {
  homogeneous,
  mixed_buffer_usage,
  mixed_memory_usage,
  realistic,
};

struct ResourceProperties {
  BufferUsage bufferUsage;
  MemoryUsage memoryUsage;
};

static ResourceProperties resource_properties(ResourceMix mix, uint32_t index) {
  switch (mix) {
  case ResourceMix::homogeneous:
    return {
        .bufferUsage = BufferUsage::transfer_src,
        .memoryUsage = MemoryUsage::device,
    };

  case ResourceMix::mixed_buffer_usage:
    switch (index % 6) {
    case 0:
      return {
          .bufferUsage = BufferUsage::transfer_src,
          .memoryUsage = MemoryUsage::device,
      };
    case 1:
      return {
          .bufferUsage = BufferUsage::transfer_dst,
          .memoryUsage = MemoryUsage::device,
      };
    case 2:
      return {
          .bufferUsage = BufferUsage::storage,
          .memoryUsage = MemoryUsage::device,
      };
    case 3:
      return {
          .bufferUsage = BufferUsage::uniform,
          .memoryUsage = MemoryUsage::device,
      };
    case 4:
      return {
          .bufferUsage = BufferUsage::vertex,
          .memoryUsage = MemoryUsage::device,
      };
    default:
      return {
          .bufferUsage = BufferUsage::indirect,
          .memoryUsage = MemoryUsage::device,
      };
    }

  case ResourceMix::mixed_memory_usage:
    switch (index % 5) {
    case 0:
      return {
          .bufferUsage = BufferUsage::transfer_src,
          .memoryUsage = MemoryUsage::automatic,
      };
    case 1:
      return {
          .bufferUsage = BufferUsage::transfer_src,
          .memoryUsage = MemoryUsage::device,
      };
    case 2:
      return {
          .bufferUsage = BufferUsage::transfer_src,
          .memoryUsage = MemoryUsage::mapped,
      };
    case 3:
      return {
          .bufferUsage = BufferUsage::transfer_src,
          .memoryUsage = MemoryUsage::mapped_write_sequential,
      };
    default:
      return {
          .bufferUsage = BufferUsage::transfer_src,
          .memoryUsage = MemoryUsage::mapped_incoherent,
      };
    }

  case ResourceMix::realistic:
    switch (index % 8) {
    case 0:
    case 1:
    case 2:
      // GPU resources populated through transfer operations.
      return {
          .bufferUsage = BufferUsage::storage,
          .memoryUsage = MemoryUsage::device,
      };

    case 3:
    case 4:
      return {
          .bufferUsage = BufferUsage::transfer_dst,
          .memoryUsage = MemoryUsage::device,
      };

    case 5:
      // Upload/staging allocation.
      return {
          .bufferUsage = BufferUsage::transfer_src,
          .memoryUsage = MemoryUsage::mapped_write_sequential,
      };

    case 6:
      // Readback allocation.
      return {
          .bufferUsage = BufferUsage::transfer_dst,
          .memoryUsage = MemoryUsage::mapped,
      };

    default:
      return {
          .bufferUsage = BufferUsage::uniform,
          .memoryUsage = MemoryUsage::automatic,
      };
    }

  default:
    std::unreachable();
  }
}

enum class LifetimePattern {
  sequential,
  windowed,
  overlapping,
};

static void run_memory_pool_case(Device &device,
                                 LifetimePattern lifetimePattern,
                                 ResourceMix resourceMix,
                                 uint32_t allocationCount) {
  MemoryPool pool = device.create_memory_pool();

  Vector<Buffer> buffers;
  buffers.reserve(allocationCount);

  constexpr uint32_t passCount = 64;

  for (uint32_t i = 0; i < allocationCount; ++i) {
    const uint32_t hash = i * 747796405u + 2891336453u;

    const uint64_t size = uint64_t{256} << (hash % 12);

    MemoryLifetime lifetime;

    switch (lifetimePattern) {
    case LifetimePattern::sequential:
      lifetime = {
          .begin = i,
          .end = i + 1,
      };
      break;

    case LifetimePattern::windowed: {
      const uint32_t begin = hash % passCount;
      const uint32_t duration = 1 + ((hash >> 8) % 16);

      lifetime = {
          .begin = begin,
          .end = std::min(begin + duration, passCount),
      };
      break;
    }

    case LifetimePattern::overlapping:
      lifetime = {
          .begin = 0,
          .end = passCount,
      };
      break;

    default:
      std::unreachable();
    }

    const ResourceProperties properties = resource_properties(resourceMix, i);

    buffers.push_back(pool.create_buffer(
        BufferCreateInfo{
            .size = size,
            .usage = properties.bufferUsage,
            .memory_usage = properties.memoryUsage,
        },
        lifetime));
  }

  {
    ZoneScopedN("MemoryPool::commit");
    pool.commit();
  }
}

int main() {
#ifdef NDEBUG
  fmt::println("waiting for tracy");

  while (!TracyIsConnected)
    std::this_thread::yield();

  fmt::println("tracy connected");
#endif

  Device device{{
      .swapchain = false,
  }};

  constexpr uint32_t allocationCount = 2048;

  while (true) {
    {
      ZoneScopedN("Homogeneous");
      run_memory_pool_case(device, LifetimePattern::windowed,
                           ResourceMix::homogeneous, allocationCount);
    }

    {
      ZoneScopedN("Mixed buffer usage");
      run_memory_pool_case(device, LifetimePattern::windowed,
                           ResourceMix::mixed_buffer_usage, allocationCount);
    }
    
    {
      ZoneScopedN("Mixed memory usage");
      run_memory_pool_case(device, LifetimePattern::windowed,
                           ResourceMix::mixed_memory_usage, allocationCount);
    }
    
    {
      ZoneScopedN("Realistic mix");
      run_memory_pool_case(device, LifetimePattern::windowed,
                           ResourceMix::realistic, allocationCount);
    }

    FrameMark;
  }
}

int main2() {
#ifdef NDEBUG
  fmt::println("waiting for tracy");
  while (!TracyIsConnected) {
    std::this_thread::yield();
  }
  fmt::println("tracy connected");
#endif
  Platform::start();
  {
    WindowImpl window{uvec2{800, 600}, "strobe"};

    Device device{{
#ifndef NDEBUG
        .debug_utils = true,
#endif
    }};

    Swapchain swapchain = device.create_swapchain(
        window.ptr(), {.extent = window.framebuffer_size()});

    Queue queue = device.get_queue(QueueFlags::graphics | QueueFlags::transfer |
                                   QueueFlags::present);

    MemoryPool memPool = device.create_memory_pool();

    CommandPool cmdPool = device.create_cmd_pool(queue);
    struct Frame {
      BinarySemaphore imageAvailable;
      Fence fence;

      Buffer vertex1;
      Buffer vertex1Stage;

      Buffer vertex2;
      Buffer vertex2Stage;
    };
    static constexpr uint32_t FramesInFlight = 2;
    Vector<Frame> frames{FramesInFlight};

    for (uint32_t i = 0; i < frames.size(); ++i) {
      frames[i] = {
          .imageAvailable = device.create_binary_semaphore(),
          .fence = device.create_fence(true),

          .vertex1 = memPool.create_buffer(
              BufferCreateInfo{
                  .size = 24,
                  .usage = BufferUsage::transfer_dst | BufferUsage::vertex,
              },
              MemoryLifetime{.begin = 0, .end = 1}),
          .vertex1Stage = memPool.create_buffer({
              .size = 24,
              .usage = BufferUsage::transfer_src,
              .memory_usage = MemoryUsage::mapped_write_sequential,
          }),

          .vertex2 = memPool.create_buffer(
              BufferCreateInfo{
                  .size = 24,
                  .usage = BufferUsage::transfer_dst | BufferUsage::vertex,
              },
              MemoryLifetime{.begin = 1, .end = 2}),
          .vertex2Stage = memPool.create_buffer({
              .size = 24,
              .usage = BufferUsage::transfer_src,
              .memory_usage = MemoryUsage::mapped_write_sequential,
          }),
      };
      frames[i].imageAvailable.set_name("imageAvailable");
    }

    const auto vertexSpv = utility::read_spirv("./vertex.spv");
    const auto fragmentSpv = utility::read_spirv("./fragment.spv");

    VertexShader vertex = device.create_vertex_shader({
        .spirv = vertexSpv,
        .nextStage = ShaderStage::fragment,
    });
    FragmentShader fragment = device.create_fragment_shader({
        .spirv = fragmentSpv,
    });

    uint32_t frameIndex = 0;
    window.show();
    while (!window.should_close()) {
      window.poll();

      Frame &frame = frames[frameIndex];
      frame.fence.wait_and_reset();

      SwapchainImage swapchainImage = swapchain.acquire(frame.imageAvailable);

      CommandBuffer cmd = cmdPool.alloc();
      cmd.begin();

      // vertex 1.
      float v1[6] = {
          -0.65f, 0.0f,  //
          0.65f,  0.0f,  //
          0.0f,   0.75f, //
      };
      std::memcpy(frame.vertex1Stage.ptr(), v1, sizeof(v1));
      cmd.copy_buffer(frame.vertex1, frame.vertex1Stage);
      cmd.memory_barrier({
          .srcStage = PipelineStage::transfer,
          .srcAccess = Access::transfer_write,
          .dstStage = PipelineStage::vertex_attribute_input,
          .dstAccess = Access::vertex_attribute_read,
      });

      Attachment colorAttachment0{
          .view = swapchainImage.view(),
          .loadOp = AttachmentLoadOp::clear,
          .storeOp = AttachmentStoreOp::store,
      };
      cmd.begin_rendering({
          .colorAttachments = &colorAttachment0,

      });
      uvec3 swapchainExtent = swapchainImage.image().extent();
      cmd.set_viewport({
          .width = static_cast<float>(swapchainExtent.x()),
          .height = static_cast<float>(swapchainExtent.y()),
      });
      cmd.set_scissor({
          .extent = {swapchainExtent.x(), swapchainExtent.y()},
      });
      {
        VertexBinding binding{
            .binding = 0,
            .stride = sizeof(float) * 2,
        };
        VertexAttribute attrib{
            .location = 0,
            .binding = 0,
            .format = Format::rg32_float,
            .offset = 0,
        };
        cmd.set_vertex_input(&binding, &attrib);
      }
      cmd.bind_shader(vertex);
      cmd.bind_shader(fragment);

      cmd.bind_vertex_buffer(frame.vertex1);
      cmd.draw(3);

      cmd.end_rendering();

      // second pass
      cmd.memory_barrier({
          .srcStage = PipelineStage::vertex_attribute_input,
          .srcAccess = Access::vertex_attribute_read,
          .dstStage = PipelineStage::transfer,
          .dstAccess = Access::transfer_write,
      });

      // vertex 2.
      float v2[6] = {
          -0.65f, 0.0f,   //
          0.0f,   -0.75f, //
          0.65f,  0.0f,   //
      };
      std::memcpy(frame.vertex2Stage.ptr(), v2, sizeof(v2));
      cmd.copy_buffer(frame.vertex2, frame.vertex2Stage);
      cmd.memory_barrier({
          .srcStage = PipelineStage::transfer,
          .srcAccess = Access::transfer_write,
          .dstStage = PipelineStage::vertex_attribute_input,
          .dstAccess = Access::vertex_attribute_read,
      });

      Attachment colorAttachment1{
          .view = swapchainImage.view(),
          .loadOp = AttachmentLoadOp::load,
          .storeOp = AttachmentStoreOp::store,
      };
      cmd.begin_rendering({
          .colorAttachments = &colorAttachment1,
      });
      cmd.set_viewport({
          .width = static_cast<float>(swapchainExtent.x()),
          .height = static_cast<float>(swapchainExtent.y()),
      });
      cmd.set_scissor({
          .extent = {swapchainExtent.x(), swapchainExtent.y()},
      });
      {
        VertexBinding binding{
            .binding = 0,
            .stride = sizeof(float) * 2,
        };
        VertexAttribute attrib{
            .location = 0,
            .binding = 0,
            .format = Format::rg32_float,
            .offset = 0,
        };
        cmd.set_vertex_input(&binding, &attrib);
      }
      cmd.bind_shader(vertex);
      cmd.bind_shader(fragment);
      cmd.bind_vertex_buffer(frame.vertex2);
      cmd.draw(3);

      cmd.end_rendering();

      cmd.end();

      BinarySemaphoreSubmitInfo wait{
          .semaphore = frame.imageAvailable,
      };
      BinarySemaphoreSubmitInfo signal{
          .semaphore = swapchainImage.presentReady(),
      };
      queue.submit({
          .cmds = {&cmd, 1},
          .wait = {&wait, 1},
          .signal = {&signal, 1},
          .fence = frame.fence,
      });

      queue.present(std::move(swapchainImage));
      frameIndex = (frameIndex + 1) % frames.size();
      FrameMark;
    }
  }
  Platform::stop();
  return 0;
}
