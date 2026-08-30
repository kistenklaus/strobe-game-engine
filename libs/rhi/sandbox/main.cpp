
#include "io.hpp"
#include "strobe/core/containers/vector.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/window/window_impl.hpp"
#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>
#include <unistd.h>
#include <vulkan/vulkan_core.h>

#include "strobe/rhi/context/context.hpp"


using namespace strobe;
using namespace strobe::window;
using namespace strobe::rhi;

int main() {
// #ifdef STROBE_TRACY
//   fmt::println("waiting for tracy");
//   while (!TracyIsConnected) {
//     std::this_thread::yield();
//   }
//   fmt::println("tracy connected");
// #endif
//
//   Platform::start([]() {
//     WindowImpl window{uvec2{800, 600}, "strobe"};
//
//     Device device{{
//         // TODO: test release builds with validation
//         // #ifndef NDEBUG
//         .debug_utils = false,
//         // #endif
//     }};
//
//     Swapchain swapchain = device.create_swapchain(
//         window.ptr(), {
//                           .extent = window.framebuffer_size(),
//                           .imageUsage = ImageUsage::color_attachment,
//                           .vsync = false,
//                       });
//
//     Queue queue = device.get_queue(QueueFlags::graphics | QueueFlags::transfer |
//                                    QueueFlags::present);
//
//     MemoryPool memPool = device.create_memory_pool();
//
//     Buffer aabbBuffer = memPool.create_buffer({
//         .size = sizeof(Aabb),
//         .bufferUsage = BufferUsage::acceleration_structure_build_input,
//         .memoryUsage = MemoryUsage::mapped_write_sequential,
//     });
//
//     aabbBuffer.set_name("aabb-buffer");
//     aabbBuffer.commit();
//     Aabb aabb{
//         .minX = -1,
//         .minY = -1,
//         .minZ = -1,
//         .maxX = 1,
//         .maxY = 1,
//         .maxZ = 1,
//     };
//     std::memcpy(aabbBuffer.ptr(), &aabb, sizeof(Aabb));
//
//     AabbGeometryData aabbGeometry{
//         .maxAabbs = 1,
//         .buffer = std::move(aabbBuffer),
//     };
//
//     Blas blas = memPool.create_blas({
//         .flags = BuildFlags::prefer_fast_trace,
//         .geometries = &aabbGeometry,
//     });
//
//     CommandPool cmdPool = device.create_cmd_pool(queue);
//
//     struct Frame {
//       BinarySemaphore imageAvailable;
//       Fence fence;
//       Buffer vertex1;
//       Buffer vertex2;
//     };
//
//     static constexpr uint32_t FramesInFlight = 2;
//     Vector<Frame> frames{FramesInFlight};
//
//     for (uint32_t i = 0; i < frames.size(); ++i) {
//       frames[i] = {
//           .imageAvailable = device.create_binary_semaphore(),
//           .fence = device.create_fence(true),
//           .vertex1 = memPool.create_buffer(
//               BufferInfo{
//                   .size = 24,
//                   .bufferUsage =
//                       BufferUsage::transfer_dst | BufferUsage::vertex,
//               },
//               MemoryLifetime{.begin = 0, .end = 1}),
//           .vertex2 = memPool.create_buffer(
//               BufferInfo{
//                   .size = 24,
//                   .bufferUsage =
//                       BufferUsage::transfer_dst | BufferUsage::vertex,
//               },
//               MemoryLifetime{.begin = 1, .end = 2}),
//       };
//       frames[i].imageAvailable.set_name("imageAvailable");
//     }
//
//     const auto vertexSpv = utility::read_spirv("./vertex.spv");
//     const auto fragmentSpv = utility::read_spirv("./fragment.spv");
//
//     VertexShader vertex = device.create_vertex_shader({
//         .spirv = vertexSpv,
//     });
//     FragmentShader fragment = device.create_fragment_shader({
//         .spirv = fragmentSpv,
//     });
//
//     uint32_t frameIndex = 0;
//     window.show();
//     while (!window.should_close()) {
//       window.poll();
//
//       Frame &frame = frames[frameIndex];
//       frame.fence.wait_and_reset();
//
//       SwapchainImage swapchainImage = swapchain.acquire(frame.imageAvailable);
//
//       CommandBuffer cmd = cmdPool.alloc();
//       cmd.begin();
//
//       if (frameIndex == 0) {
//         BuildRangeInfo range{
//             .primitiveCount = 1,
//         };
//         assert(blas);
//         cmd.build(blas, &range);
//       }
//
//       // vertex 1.
//       vec2 v1[3] = {
//           {-0.65f, 0.0f}, //
//           {0.65f, 0.0f},  //
//           {0.0f, 0.75f},  //
//       };
//       cmd.update(frame.vertex1, v1, sizeof(v1));
//
//       cmd.memory_barrier(access::transfer_write, access::vertex_attribute_read);
//
//       Attachment colorAttachment0{
//           .view = swapchainImage.view(),
//           .loadOp = AttachmentLoadOp::clear,
//           .storeOp = AttachmentStoreOp::store,
//       };
//       cmd.begin_rendering({
//           .colorAttachments = &colorAttachment0,
//       });
//       { // setup rendering state
//         const uvec2 swapchainExtent = swapchainImage.extent();
//         cmd.set_viewport({
//             .extent = swapchainExtent,
//         });
//         cmd.set_scissor({
//             .extent = swapchainExtent,
//         });
//         {
//           VertexBinding binding{
//               .binding = 0,
//               .stride = sizeof(float) * 2,
//           };
//           VertexAttribute attrib{
//               .location = 0,
//               .binding = 0,
//               .format = Format::rg32_float,
//               .offset = 0,
//           };
//           cmd.set_vertex_input(&binding, &attrib);
//         }
//         cmd.bind_shader(vertex);
//         cmd.bind_shader(fragment);
//       }
//
//       { // first draw
//         cmd.bind_vertex_buffer(frame.vertex1);
//         cmd.draw(3);
//       }
//
//       cmd.end_rendering();
//
//       // second pass
//       cmd.memory_barrier(access::vertex_attribute_read, access::transfer_write);
//
//       // vertex 2.
//       vec2 v2[3] = {
//           {-0.65f, 0.0f}, //
//           {0.0f, -0.75f}, //
//           {0.65f, 0.0f}   //
//       };
//       cmd.update<vec2>(frame.vertex2, v2);
//
//       cmd.memory_barrier(
//           access::transfer_write | access::color_attachment_write,
//           access::vertex_attribute_read | access::color_attachment_read_write);
//
//       Attachment colorAttachment1{
//           .view = swapchainImage.view(),
//           .loadOp = AttachmentLoadOp::load,
//           .storeOp = AttachmentStoreOp::store,
//       };
//       cmd.begin_rendering({
//           .colorAttachments = &colorAttachment1,
//       });
//
//       {
//         cmd.bind_vertex_buffer(frame.vertex2);
//         cmd.draw(3);
//       }
//       cmd.end_rendering();
//
//       cmd.end();
//
//       BinarySemaphoreSubmitInfo wait{
//           .semaphore = frame.imageAvailable,
//           .stage = PipelineStage::all_commands,
//       };
//       BinarySemaphoreSubmitInfo signal{
//           .semaphore = swapchainImage.presentReady(),
//           .stage = PipelineStage::all_commands,
//       };
//       queue.submit({
//           .cmds = &cmd,
//           .wait = &wait,
//           .signal = &signal,
//           .fence = frame.fence,
//       });
//
//       queue.present(std::move(swapchainImage));
//       frameIndex = (frameIndex + 1) % frames.size();
//       FrameMark;
//     }
//   });
}
