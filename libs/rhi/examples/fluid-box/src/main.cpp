#include "apply-force.slang.spv.hpp"
#include "splat-frag.slang.spv.hpp"
#include "splat-vert.slang.spv.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/platform/window.hpp"
#include "strobe/platform/window_events.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/rhi.hpp"
#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/attachment_load_op.hpp"
#include "strobe/rhi/types/blend_equation.hpp"
#include "strobe/rhi/types/blend_factor.hpp"
#include "strobe/rhi/types/blend_op.hpp"
#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/color_component.hpp"
#include "strobe/rhi/types/format.hpp"
#include "strobe/rhi/types/image_aspect.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"
#include "strobe/rhi/types/primitive_topology.hpp"
#include "strobe/rhi/types/sampler_address_mode.hpp"
#include "strobe/rhi/types/vertex_input_rate.hpp"
#include "tf-frag.slang.spv.hpp"
#include "tf-vert.slang.spv.hpp"
#include "tracy/Tracy.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <fmt/ostream.h>
#include <random>
#include <vulkan/vulkan_core.h>

using namespace strobe;

static constexpr uint32_t FRAMES_IN_FLIGHT = 2;
static constexpr size_t PARTICLE_COUNT = 1 << 20;
static constexpr float PARTICLE_MASS = 1;
static constexpr float LINEAR_DAMPING = 0.5f;
static constexpr float RESTITUTION = 1.0f; // [0,1]
static constexpr vec2 GRAVITY = vec2(0, 100);
static constexpr float WINDOW_COUPLING = 1.0f; // 1/s

static constexpr float SIM_DELTA_TIME_MS = 1.0f; // 1ms
static constexpr float PARTICLE_VIS_RADIUS = 4.0f;

static std::atomic<vec2> g_windowPos;
static std::atomic<vec2> g_windowExtent;

// Simulation space is window-local.
static vec2 g_viewportPos{0.0f};
static vec2 g_viewportExtent;

// Motion of the physical container through desktop space.
static vec2 g_previousWindowPos;
static vec2 g_windowVelocity;
static vec2 g_windowAcceleration;

static vec2 g_externalForce;

static std::atomic<vec2> g_mousePos;

static constexpr size_t APPLY_FORCE_WG_SIZE = 256;

struct Particle {
  vec2 position;
  vec2 velocity;
};
static std::array<Particle, PARTICLE_COUNT> g_particles;

static void fluid_simulation_init() {
  g_viewportPos = vec2{0.0f};
  g_viewportExtent = g_windowExtent.load(std::memory_order_relaxed);

  g_previousWindowPos = g_windowPos.load(std::memory_order_relaxed);

  g_windowVelocity = vec2{0.0f};
  g_windowAcceleration = vec2{0.0f};

  std::mt19937 prng{std::random_device{}()};

  std::uniform_real_distribution<float> xdist{0.0f, g_viewportExtent.x()};
  std::uniform_real_distribution<float> ydist{0.0f, g_viewportExtent.y()};
  std::uniform_real_distribution<float> udist{0.0f, 1.0f};

  constexpr float speed = 100.0f;

  for (Particle &particle : g_particles) {
    particle.position = vec2{
        xdist(prng),
        ydist(prng),
    };
    float theta = 2.0f * std::numbers::pi_v<float> * udist(prng);
    particle.velocity = speed * vec2{
                                    std::cos(theta),
                                    std::sin(theta),
                                };
  }
}

static void update_external_acceleration(float dt) {
  g_viewportPos = vec2{0.0f};
  g_viewportExtent = g_windowExtent.load(std::memory_order_relaxed);
  const vec2 windowPos = g_windowPos.load(std::memory_order_relaxed);
  const vec2 windowDisplacement = windowPos - g_previousWindowPos;
  const vec2 windowVelocity = windowDisplacement / dt;
  g_externalForce = WINDOW_COUPLING * windowVelocity + GRAVITY;
  g_previousWindowPos = windowPos;
}

static void upload_particles(rhi::CommandBuffer cmd, rhi::Buffer particles) {
  const size_t size_bytes = g_particles.size() * sizeof(Particle);
  assert(dst.size() >= size);
  cmd.update({particles}, g_particles.data(), size_bytes);
}

static void apply_forces(rhi::CommandBuffer &cmd,
                         rhi::ResourceDescriptor &particles,
                         rhi::ComputeShader &shader) {

  struct PushConstants {
    uint32_t particleCount;        // 0
    float deltaTime;               // 4
    float mass;                    // 8
    float linearDamping;           // 12
    float restitution;             // 16
    uint32_t padding;              // 20
    alignas(8) vec2 externalForce; // 24
    vec2 viewportPos;              // 32
    vec2 viewportExtent;           // 40
    // uint32_t particleIndex;        // 48
  };
  static_assert(offsetof(PushConstants, particleCount) == 0);
  static_assert(offsetof(PushConstants, deltaTime) == 4);
  static_assert(offsetof(PushConstants, mass) == 8);
  static_assert(offsetof(PushConstants, linearDamping) == 12);
  static_assert(offsetof(PushConstants, restitution) == 16);
  static_assert(offsetof(PushConstants, externalForce) == 24);
  static_assert(offsetof(PushConstants, viewportPos) == 32);
  static_assert(offsetof(PushConstants, viewportExtent) == 40);

  PushConstants pc{
      .particleCount = PARTICLE_COUNT,
      .deltaTime = SIM_DELTA_TIME_MS * 1e-3f,
      .mass = PARTICLE_MASS,
      .linearDamping = LINEAR_DAMPING,
      .restitution = RESTITUTION,
      .padding = 0,
      .externalForce = g_externalForce,
      .viewportPos = g_viewportPos,
      .viewportExtent = g_viewportExtent,
  };

  cmd.push(0, &pc, sizeof(pc));
  cmd.push(48, particles);
  const uint32_t wgCount =
      (PARTICLE_COUNT + APPLY_FORCE_WG_SIZE - 1) / APPLY_FORCE_WG_SIZE;
  cmd.bind_shader(shader);
  cmd.dispatch(wgCount);
}

static void splat_particles(rhi::CommandBuffer &cmd, rhi::ImageView &target,
                            const rhi::Buffer &particlePositions,
                            const rhi::Buffer &unitCircleVerticies,
                            const rhi::VertexShader &vertexShader,
                            const rhi::FragmentShader &fragmentShader) {
  // precondition: target is in attachment layout!

  rhi::Attachment attachment{
      .view = target,
      .loadOp = rhi::AttachmentLoadOp::clear,
      .storeOp = rhi::AttachmentStoreOp::store,
  };
  cmd.begin_rendering({.colorAttachments = &attachment});

  cmd.set_viewport({.extent = target.extent().xy()});
  cmd.set_scissor({.extent = target.extent().xy()});

  cmd.set_color_blend_enable(0, 1, true);
  rhi::BlendEquation blendEquation{
      .srcColor = rhi::BlendFactor::one,
      .dstColor = rhi::BlendFactor::one,
      .colorBlendOp = rhi::BlendOp::add,
      .srcAlpha = rhi::BlendFactor::one,
      .dstAlpha = rhi::BlendFactor::one,
      .alphaBlendOp = rhi::BlendOp::add,
  };
  cmd.set_color_blend_equation(0, &blendEquation);
  rhi::ColorComponent writeMask = rhi::ColorComponent::r;
  cmd.set_color_write_mask(0, &writeMask);

  std::array bindings = {
      rhi::VertexBinding{
          .binding = 0,
          .stride = sizeof(vec2), // unit-circle vertex
          .inputRate = rhi::VertexInputRate::vertex,
          .divisor = 1,
      },
      rhi::VertexBinding{
          .binding = 1,
          .stride = sizeof(Particle), // particle center
          .inputRate = rhi::VertexInputRate::instance,
          .divisor = 1,
      },
  };
  std::array attributes = {
      rhi::VertexAttribute{
          .location = 0,
          .binding = 0,
          .format = rhi::Format::rg32_float,
          .offset = 0,
      },
      rhi::VertexAttribute{
          .location = 1, // <-- must be different
          .binding = 1,
          .format = rhi::Format::rg32_float,
          .offset = offsetof(Particle, position),
      },
  };

  cmd.set_vertex_input(bindings, attributes);
  std::array vertexBuffers = {
      rhi::BufferOffset{
          .buffer = unitCircleVerticies,
      },
      rhi::BufferOffset{
          .buffer = particlePositions,
      },
  };
  cmd.bind_vertex_buffers(vertexBuffers);
  cmd.set_primitive_topology(rhi::PrimitiveTopology::triangle_fan);

  cmd.bind_shader(vertexShader);
  cmd.bind_shader(fragmentShader);

  struct PushConstant {
    vec2 viewportPosition;
    vec2 viewportExtent;
    float radius;
  };
  PushConstant pc{
      .viewportPosition = g_viewportPos,
      .viewportExtent = g_viewportExtent,
      .radius = PARTICLE_VIS_RADIUS,
  };
  cmd.push(0, &pc, sizeof(pc));

  cmd.draw(unitCircleVerticies.size() / sizeof(vec2), PARTICLE_COUNT);

  cmd.end_rendering();
}

static void render_transfer_function(rhi::CommandBuffer &cmd,
                                     rhi::ImageView target,
                                     rhi::ResourceDescriptor &densityMap,
                                     rhi::SamplerDescriptor &densityMapSampler,
                                     rhi::VertexShader &vertexShader,
                                     rhi::FragmentShader &fragmentShader) {
  // precondition: density map is a image descriptor
  // precondition: target is in attachment layout.

  rhi::Attachment attachment{
      .view = target,
      .loadOp = rhi::AttachmentLoadOp::clear,
      .storeOp = rhi::AttachmentStoreOp::store,
  };
  cmd.begin_rendering({.colorAttachments = &attachment});

  cmd.set_viewport({.extent = target.extent().xy()});
  cmd.set_scissor({.extent = target.extent().xy()});

  cmd.set_color_blend_enable(0, 1, false);
  rhi::ColorComponent writeMask = rhi::ColorComponent::rgba;
  cmd.set_color_write_mask(0, &writeMask);
  cmd.set_vertex_input({}, {});
  cmd.set_primitive_topology(rhi::PrimitiveTopology::triangle_list);
  cmd.bind_shader(vertexShader);
  cmd.bind_shader(fragmentShader);

  cmd.push(0, densityMap);
  cmd.push(4, densityMapSampler);

  cmd.draw(3);

  cmd.end_rendering();
}

static std::atomic<bool> g_should_close = false;

static void on_window_close(void *,
                            const platform::WindowCloseEvent &) noexcept {
  g_should_close.store(true, std::memory_order_relaxed);
}

static void
on_window_move(void *, const platform::WindowPositionEvent &event) noexcept {
  g_windowPos.store(
      vec2{static_cast<float>(event.x()), static_cast<float>(event.y())},
      std::memory_order_relaxed);
}

static void on_window_resize(void *,
                             const platform::WindowSizeEvent &event) noexcept {
  g_windowExtent.store(vec2{static_cast<float>(event.width()),
                            static_cast<float>(event.height())},
                       std::memory_order_relaxed);
}

static void on_mouse_move(void *,
                          const platform::CursorPositionEvent &event) noexcept {
  g_mousePos.store(
      vec2{
          static_cast<float>(event.x()),
          static_cast<float>(event.y()),
      },
      std::memory_order_relaxed);
}

static constexpr size_t UNIT_CIRCLE_POINTS = 10;

static rhi::Buffer create_unit_circle(rhi::Device &device) {
  constexpr size_t N = UNIT_CIRCLE_POINTS;
  std::array<vec2, N + 2> vertices;

  // Triangle-fan center.
  vertices[0] = vec2{0.0f, 0.0f};

  // Circumscribed regular N-gon: every edge is tangent to the unit circle.
  constexpr float pi = std::numbers::pi_v<float>;
  const float radius = 1.0f / std::cos(pi / static_cast<float>(N));

  for (size_t i = 0; i <= N; ++i) {
    const float theta =
        2.0f * pi * static_cast<float>(i % N) / static_cast<float>(N);
    vertices[i + 1] = vec2{
        radius * std::cos(theta),
        radius * std::sin(theta),
    };
  }

  constexpr size_t size_bytes = sizeof(vertices);

  auto buf = device.create_buffer({
      .size = size_bytes,
      .bufferUsage = rhi::BufferUsage::vertex | rhi::BufferUsage::transfer_dst,
      .memoryUsage = rhi::MemoryUsage::automatic,
  });

  device.async_upload({buf}, vertices.data(), size_bytes).wait();
  return buf;
}

int main() {

  platform::Window window{{
      .size = uvec2(800, 800),
      .title = "FLOATING",
  }};

  auto close_listener = window.add_window_close_listener(
      EventListenerRef<platform::WindowCloseEvent>::fromNative(
          nullptr, on_window_close));

  auto pos_listener = window.add_window_position_listener(
      EventListenerRef<platform::WindowPositionEvent>::fromNative(
          nullptr, on_window_move));
  auto size_listener = window.add_window_size_listener(
      EventListenerRef<platform::WindowSizeEvent>::fromNative(
          nullptr, on_window_resize));

  auto mouse_move_listener = window.add_cursor_position_listener(
      EventListenerRef<platform::CursorPositionEvent>::fromNative(
          nullptr, on_mouse_move));

  rhi::Device device = rhi::create_device({
      .debug_utils = true,
  });

  rhi::Queue queue = device.get_queue();
  rhi::CommandPool cmdpool = device.create_cmdpool();

  rhi::Swapchain swapchain = device.create_swapchain({
      .window = window.window_ptr(),
      .vsync = false,
  });

  window.resizable(true);
  window.visible(true);

  g_windowPos = window.position();
  g_windowExtent = window.size();

  rhi::SamplerDescriptor linearSampler = device.create_sampler_descriptor({
      .magFilter = rhi::Filter::linear,
      .minFilter = rhi::Filter::linear,
      .addressModeU = rhi::SamplerAddressMode::clamp_to_edge,
      .addressModeV = rhi::SamplerAddressMode::clamp_to_edge,
      .addressModeW = rhi::SamplerAddressMode::clamp_to_edge,
  });

  rhi::Buffer unitCircleVerticies = create_unit_circle(device);

  rhi::VertexShader splatVertexShader = device.create_vertex_shader({
      .spirv = splat_vert_slang_spv,
      .nextStage = rhi::ShaderStage::fragment,
  });
  rhi::FragmentShader splatFragmentShader = device.create_fragment_shader({
      .spirv = splat_frag_slang_spv,
  });

  rhi::VertexShader tfVertexShader = device.create_vertex_shader({
      .spirv = tf_vert_slang_spv,
      .nextStage = rhi::ShaderStage::fragment,
  });
  rhi::FragmentShader tfFragmentShader = device.create_fragment_shader({
      .spirv = tf_frag_slang_spv,
  });

  rhi::ComputeShader applyForceShader = device.create_compute_shader({
      .spirv = apply_force_slang_spv,
  });

  fluid_simulation_init();

  constexpr size_t size_bytes = g_particles.size() * sizeof(Particle);
  rhi::Buffer particles = device.create_buffer({
      .size = size_bytes,
      .bufferUsage = rhi::BufferUsage::transfer_dst | rhi::BufferUsage::vertex |
                     rhi::BufferUsage::storage,
      .memoryUsage = rhi::MemoryUsage::automatic,
  });
  auto cmd = cmdpool.alloc();
  cmd.begin();
  upload_particles(cmd, particles);
  cmd.memory_barrier({
      .srcStage = rhi::PipelineStage::transfer,
      .srcAccess = rhi::Access::transfer_write,
      .dstStage = rhi::PipelineStage::compute_shader |
                  rhi::PipelineStage::vertex_attribute_input,
      .dstAccess = rhi::Access::shader_storage_read |
                   rhi::Access::shader_storage_write |
                   rhi::Access::vertex_attribute_read,
  });
  cmd.end();
  queue.submit(&cmd).wait();

  rhi::ResourceDescriptor particleDescriptor =
      device.create_storage_buffer_descriptor({
          .buffer = particles,
      });

  struct Frame {
    rhi::Timepoint ready;
    rhi::Image densityMapImage;
    rhi::ImageView densityMapView;
    rhi::ResourceDescriptor densityMapDescriptor;
  };

  std::array<Frame, FRAMES_IN_FLIGHT> frames;
  uint32_t frameIndex = 0;
  auto epoch = std::chrono::high_resolution_clock::now();

  auto get_time = [&] noexcept -> float {
    auto dur_since_epoch = std::chrono::high_resolution_clock::now() - epoch;
    return std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
               dur_since_epoch)
        .count();
  };

  float last = get_time();
  while (!g_should_close.load(std::memory_order_relaxed)) {

    Frame &frame = frames[frameIndex];
    frame.ready.wait();

    rhi::SwapchainImage image = swapchain.acquire();
    assert(image);

    // resize image.
    if (!frame.densityMapImage ||
        frame.densityMapImage.extent().xy() != image.extent()) {
      frame.densityMapImage = device.create_image({
          .type = rhi::ImageType::image_2d,
          .format = rhi::Format::r32_float,
          .extent = uvec3(image.extent().x(), image.extent().y(), 1),
          .mip_levels = 1,
          .arrayLayers = 1,
          .samples = rhi::SampleCount::x1,
          .linearTiling = false,
          .imageUsage =
              rhi::ImageUsage::sampled | rhi::ImageUsage::color_attachment,
          .memoryUsage = rhi::MemoryUsage::automatic,
      });
      frame.densityMapDescriptor = device.create_sampled_image_descriptor({
          .image = frame.densityMapImage,
          .layout = rhi::ImageLayout::read_only,
          .viewType = rhi::ImageViewType::image_2d,
          .format = frame.densityMapImage.format(),
          .subresource = {.aspect = rhi::ImageAspect::color},
      });
    }
    if (!frame.densityMapView ||
        frame.densityMapView.extent() != frame.densityMapImage.extent()) {
      frame.densityMapView = device.create_image_view(
          frame.densityMapImage,
          {
              .type = rhi::ImageViewType::image_2d,
              .format = frame.densityMapImage.format(),
              .range = {.aspect = rhi::ImageAspect::color},
          });
    }

    rhi::CommandBuffer cmd = cmdpool.alloc();
    cmd.begin();

    cmd.memory_barrier(rhi::MemoryBarrier{
        .srcStage = rhi::PipelineStage::vertex_attribute_input,
        .srcAccess = rhi::Access::vertex_attribute_read,
        .dstStage = rhi::PipelineStage::compute_shader,
        .dstAccess = rhi::Access::shader_storage_read |
                     rhi::Access::shader_storage_write,
    });

    const float now = get_time();
    const float delta = now - last;

    const uint32_t steps = static_cast<uint32_t>(delta / SIM_DELTA_TIME_MS);

    if (steps > 0) {
      const float dt = steps * SIM_DELTA_TIME_MS * 1e-3f;
      update_external_acceleration(dt);
    }
    for (uint32_t i = 0; i < steps; ++i) {
      if (i != 0) {
        cmd.memory_barrier(rhi::MemoryBarrier{
            .srcStage = rhi::PipelineStage::compute_shader,
            .srcAccess = rhi::Access::shader_storage_read |
                         rhi::Access::shader_storage_write,
            .dstStage = rhi::PipelineStage::compute_shader,
            .dstAccess = rhi::Access::shader_storage_read |
                         rhi::Access::shader_storage_write,
        });
      }
      apply_forces(cmd, particleDescriptor, applyForceShader);
    }
    last += steps * SIM_DELTA_TIME_MS;

    cmd.memory_barrier(rhi::MemoryBarrier{
        .srcStage = rhi::PipelineStage::compute_shader,
        .srcAccess = rhi::Access::shader_storage_write,
        .dstStage = rhi::PipelineStage::vertex_attribute_input,
        .dstAccess = rhi::Access::vertex_attribute_read,
    });
    cmd.transition_image(frame.densityMapImage, rhi::ImageLayout::undefined,
                         rhi::ImageLayout::attachment);
    splat_particles(cmd, frame.densityMapView, particles, unitCircleVerticies,
                    splatVertexShader, splatFragmentShader);

    cmd.transition_image(frame.densityMapImage, rhi::ImageLayout::attachment,
                         rhi::ImageLayout::read_only);

    cmd.transition_image(image.image(), rhi::ImageLayout::undefined,
                         rhi::ImageLayout::attachment);

    render_transfer_function(cmd, image.view(), frame.densityMapDescriptor,
                             linearSampler, tfVertexShader, tfFragmentShader);

    cmd.transition_image(image.image(), rhi::ImageLayout::attachment,
                         rhi::ImageLayout::present);

    cmd.end();

    queue.wait(image);
    frame.ready = queue.submit(&cmd);
    frame.ready.wait();
    queue.present(std::move(image));

    frameIndex = frameIndex + 1;
    if (frameIndex == FRAMES_IN_FLIGHT) {
      frameIndex = 0;
    }
    FrameMark;
  }
}
