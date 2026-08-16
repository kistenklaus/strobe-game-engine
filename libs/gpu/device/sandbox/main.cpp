
#include "io.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/fragment_shader.hpp"
#include "strobe/gpu/device/image_memory_barrier.hpp"
#include "strobe/gpu/device/queue_flags.hpp"
#include "strobe/gpu/vulkan/binary_semaphore.hpp"
#include "strobe/gpu/vulkan/cmd/barrier.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
#include "strobe/gpu/vulkan/command_pool.hpp"
#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/vulkan/context/create_info.hpp"

#include "strobe/gpu/vulkan/fence.hpp"
#include "strobe/gpu/vulkan/image_view.hpp"
#include "strobe/gpu/vulkan/queue.hpp"
#include "strobe/gpu/vulkan/surface.hpp"
#include "strobe/gpu/vulkan/swapchain.hpp"
#include "strobe/window/window_impl.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

using namespace strobe;
using namespace strobe::window;
using namespace strobe::gpu;

int main() {
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

    CommandPool cmdPool = device.create_cmd_pool(queue);
    struct Frame {
      BinarySemaphore imageAvailable;
      Fence fence;
    };
    static constexpr uint32_t FramesInFlight = 2;
    Vector<Frame> frames{FramesInFlight};

    for (uint32_t i = 0; i < frames.size(); ++i) {
      frames[i] = {
          .imageAvailable = device.create_binary_semaphore(),
          .fence = device.create_fence(true),
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

      Attachment colorAttachment{
          .view = swapchainImage.view(),
      };
      cmd.begin_rendering({
          .colorAttachments = {&colorAttachment, 1},

      });

      uvec3 swapchainExtent = swapchainImage.image().extent();
      cmd.set_viewport({
          .width = static_cast<float>(swapchainExtent.x()),
          .height = static_cast<float>(swapchainExtent.y()),
      });
      cmd.set_scissor({
          .extent = {swapchainExtent.x(), swapchainExtent.y()},
      });

      cmd.bind_shader(vertex);
      cmd.bind_shader(fragment);
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

int main2() {

  Platform::start();
  {

    WindowImpl window{uvec2{800, 600}, "strobe"};

    vulkan::QueueDescription queueDesc{
        .require = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT |
                   VK_QUEUE_TRANSFER_BIT,
        .present = vulkan::required,
    };

    vulkan::ContextCreateInfo createInfo{
        .debug_utils = vulkan::feature::required,
        .surface = vulkan::feature::required,
        .swapchain = vulkan::feature::required,
        .timeline_semaphore = vulkan::feature::required,
        .queue_count = 1,
        .pQueues = &queueDesc,
    };

    vulkan::Context context(createInfo);

    vulkan::Surface surface = vulkan::create_surface(&context, window.ptr());

    vulkan::Queue queue = context.queue(0);

    VkSurfaceFormatKHR swapchainFormat{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    vulkan::Swapchain swapchain = vulkan::create_swapchain(
        &context,
        vulkan::SwapchainInfo{
            .surface = surface,
            .minImageCount = 3,
            .format = swapchainFormat,
            .extent = window.size(),
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .queueFamilyIndicies = span<const uint32_t>{&queue.family, 1},
            .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
        });

    Vector<vulkan::Image> swapchainImages;
    swapchainImages.resize(
        vulkan::get_swapchain_images(&context, swapchain, swapchainImages));
    vulkan::get_swapchain_images(&context, swapchain, swapchainImages);

    fmt::println("swapchain-size: {}", swapchainImages.size());

    Vector<vulkan::ImageView> swapchainViews{swapchainImages.size()};
    for (uint32_t i = 0; i < swapchainImages.size(); ++i) {
      swapchainViews[i] = vulkan::create_image_view(
          &context, vulkan::ImageViewInfo{
                        .image = swapchainImages[i],
                        .format = swapchainFormat.format,
                    });
    }

    window.show();

    vulkan::CommandPool cmdPool = vulkan::create_command_pool(
        &context, {
                      .queue = queue,
                      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                  });

    struct Frame {
      vulkan::CommandBuffer cmd;
      vulkan::BinarySemaphore imageAvailable;
      vulkan::Fence fence;
    };

    static uint32_t FramesInFlight = 2;
    Vector<Frame> frames{FramesInFlight};
    for (uint32_t i = 0; i < frames.size(); ++i) {
      frames[i] = {
          .cmd = vulkan::alloc_command_buffer(&context, {.pool = cmdPool}),
          .imageAvailable = vulkan::create_binary_semaphore(&context),
          .fence = vulkan::create_fence(
              &context, {.flags = VK_FENCE_CREATE_SIGNALED_BIT}),
      };
    }

    Vector<vulkan::BinarySemaphore> presentReady{swapchainImages.size()};

    for (auto &semaphore : presentReady) {
      semaphore = vulkan::create_binary_semaphore(&context);
    }

    using Clock = std::chrono::steady_clock;

    auto to_us = [](Clock::duration d) {
      return std::chrono::duration<float, std::micro>(d).count();
    };

    uint32_t frameIndex = 0;
    uint64_t frameCounter = 0;

    bool profile = true;

    while (!window.should_close()) {
      window.poll();

      const auto frameStart = Clock::now();

      Frame &frame = frames[frameIndex];

      const auto t0 = Clock::now();

      vulkan::wait_for_fence(&context, frame.fence);

      const auto t1 = Clock::now();

      vulkan::reset_fence(&context, frame.fence);

      vulkan::SwapchainAcquireResult result =
          vulkan::acquire_next_swapchain_image(
              &context, swapchain,
              vulkan::SwapchainAcquireInfo{
                  .signalSemaphore = frame.imageAvailable,
              });

      const auto t2 = Clock::now();

      if (result.status != vulkan::SwapchainAcquireStatus::success) {
        throw std::runtime_error("will fix recreation later");
      }

      const uint32_t imageIndex = result.imageIndex;
      vulkan::Image image = swapchainImages[imageIndex];

      vulkan::reset_command_buffer(frame.cmd);

      vulkan::CommandBuffer cmd = frame.cmd;

      vulkan::begin_command_buffer(cmd);

      {
        vulkan::ImageMemoryBarrier imageBarrier{
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = image,
        };

        vulkan::cmd_pipeline_barrier(
            cmd,
            {
                .imageBarriers =
                    span<const vulkan::ImageMemoryBarrier>{&imageBarrier, 1},
            });
      }

      vulkan::end_command_buffer(cmd);

      const auto t3 = Clock::now();

      {
        vulkan::BinarySemaphoreSubmitInfo wait{
            .semaphore = frame.imageAvailable,
            .stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        };

        vulkan::BinarySemaphoreSubmitInfo signal{
            .semaphore = presentReady[imageIndex],
            .stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        };

        vulkan::queue_submit(
            queue,
            vulkan::SubmitInfo{
                .waitBinarySemaphores =
                    span<const vulkan::BinarySemaphoreSubmitInfo>{&wait, 1},
                .command_buffers = span<const vulkan::CommandBuffer>{&cmd, 1},
                .signalBinarySemaphores =
                    span<const vulkan::BinarySemaphoreSubmitInfo>{&signal, 1},
                .fence = frame.fence,
            });
      }

      const auto t4 = Clock::now();

      vulkan::queue_present(queue, swapchain, imageIndex,
                            vulkan::PresentInfo{
                                .waitBinarySemaphores =
                                    span<const vulkan::BinarySemaphore>{
                                        &presentReady[imageIndex], 1},
                            });

      const auto t5 = Clock::now();

      frameIndex = (frameIndex + 1) % frames.size();

      const auto frameEnd = Clock::now();

      if (profile) {
        fmt::println("#{:06} img={} frame={} | "
                     "fence={:8.2f}us "
                     "acquire={:8.2f}us "
                     "record={:8.2f}us "
                     "submit={:8.2f}us "
                     "present={:8.2f}us "
                     "| total={:8.2f}us ({:7.1f} fps)",
                     frameCounter++, imageIndex, frameIndex, to_us(t1 - t0),
                     to_us(t2 - t1), to_us(t3 - t2), to_us(t4 - t3),
                     to_us(t5 - t4), to_us(frameEnd - frameStart),
                     1'000'000.0f / to_us(frameEnd - frameStart));
      }
    }

    vulkan::wait_queue_idle(queue);

    for (uint32_t i = 0; i < frames.size(); ++i) {
      vulkan::destroy_binary_semaphore(&context, frames[i].imageAvailable);
      vulkan::destroy_fence(&context, frames[i].fence);
    }

    vulkan::destroy_command_pool(&context, cmdPool);

    for (uint32_t i = 0; i < swapchainViews.size(); ++i) {
      vulkan::destroy_binary_semaphore(&context, presentReady[i]);
      vulkan::destroy_image_view(&context, swapchainViews[i]);
    }

    vulkan::destroy_swapchain(&context, swapchain);

    vulkan::destroy_surface(&context, surface);
  }
  Platform::stop();
  return 0;
}
