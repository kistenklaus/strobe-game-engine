#include "strobe/imgui/imgui.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "strobe/imgui/fragment.slang.spv.hpp"
#include "strobe/imgui/vertex.slang.spv.hpp"
#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/attachment_load_op.hpp"
#include "strobe/rhi/types/attachment_store_op.hpp"
#include "strobe/rhi/types/blend_equation.hpp"
#include "strobe/rhi/types/blend_factor.hpp"
#include "strobe/rhi/types/blend_op.hpp"
#include "strobe/rhi/types/buffer_usage.hpp"
#include "strobe/rhi/types/color_component.hpp"
#include "strobe/rhi/types/cull_mode.hpp"
#include "strobe/rhi/types/filter.hpp"
#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"
#include "strobe/rhi/types/polygon_mode.hpp"
#include "strobe/rhi/types/primitive_topology.hpp"
#include "strobe/rhi/types/sample_count.hpp"
#include "strobe/rhi/types/sampler_mipmap_mode.hpp"
#include "strobe/rhi/types/vertex_binding.hpp"
#include <cstdlib>
#include <fmt/base.h>
#include <utility>

namespace strobe::imgui {

static rhi::Device g_device;
static rhi::Buffer g_vertex_buffer{};
static rhi::Buffer g_index_buffer{};
static void *g_host_vertex_stage = nullptr;
static void *g_host_index_stage = nullptr;
static uint64_t g_vertex_capacity{0};
static uint64_t g_index_capacity{0};

static rhi::SamplerDescriptor g_linear_sampler{};
static rhi::SamplerDescriptor g_nearest_sampler{};

static rhi::VertexShader g_vertex_shader{};
static rhi::FragmentShader g_fragment_shader{};

static void callback_reset_render_state(const ImDrawList *, const ImDrawCmd *) {
}

static void callback_set_sampler_linear(const ImDrawList *, const ImDrawCmd *) {
}

static void callback_set_sampler_nearest(const ImDrawList *,
                                         const ImDrawCmd *) {}

void init(GLFWwindow *window, rhi::Device device) noexcept {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto &io = ImGui::GetIO();
  io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
  auto &platform_io = ImGui::GetPlatformIO();
  platform_io.DrawCallback_ResetRenderState = callback_reset_render_state;
  platform_io.DrawCallback_SetSamplerLinear = callback_set_sampler_linear;
  platform_io.DrawCallback_SetSamplerNearest = callback_set_sampler_nearest;

  ImGui_ImplGlfw_InitForVulkan(window, true);
  g_device = std::move(device);
  g_vertex_buffer = {};
  g_vertex_capacity = 0;
  assert(g_host_vertex_stage == nullptr);
  g_host_vertex_stage = nullptr;
  g_index_buffer = {};
  g_index_capacity = 0;
  assert(g_host_index_stage == nullptr);
  g_host_index_stage = nullptr;

  g_linear_sampler = g_device.create_sampler_descriptor({
      .magFilter = rhi::Filter::linear,
      .minFilter = rhi::Filter::linear,
      .mipmapMode = rhi::SamplerMipmapMode::linear,
      .addressModeU = rhi::SamplerAddressMode::clamp_to_edge,
      .addressModeV = rhi::SamplerAddressMode::clamp_to_edge,
      .addressModeW = rhi::SamplerAddressMode::clamp_to_edge,
  });

  g_nearest_sampler = g_device.create_sampler_descriptor({
      .magFilter = rhi::Filter::nearest,
      .minFilter = rhi::Filter::nearest,
      .mipmapMode = rhi::SamplerMipmapMode::nearest,
      .addressModeU = rhi::SamplerAddressMode::clamp_to_edge,
      .addressModeV = rhi::SamplerAddressMode::clamp_to_edge,
      .addressModeW = rhi::SamplerAddressMode::clamp_to_edge,
  });

  g_vertex_shader = g_device.create_vertex_shader({
      .spirv = strobe_imgui_vertex_slang_spv,
      .nextStage = rhi::ShaderStage::fragment,
  });

  g_fragment_shader = g_device.create_fragment_shader(
      {.spirv = strobe_imgui_fragment_slang_spv});
}

void begin_frame() noexcept {
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

struct Texture {
  rhi::Image image;
  rhi::ResourceDescriptor descriptor;
};

static void create_texture(ImTextureData *texture) {
  assert(texture);
  assert(texture->BackendUserData == nullptr);
  assert(texture->GetTexID() == ImTextureID_Invalid);

  rhi::Format format;
  switch (texture->Format) {
  case ImTextureFormat_RGBA32:
    format = rhi::Format::rgba8_unorm;
    break;

  case ImTextureFormat_Alpha8:
    fmt::println("Alpha8 ImGui textures are not supported");
    std::terminate();

  default:
    std::unreachable();
  }

  rhi::Image image = g_device.create_image({
      .type = rhi::ImageType::image_2d,
      .format = format,
      .extent =
          uvec3{
              static_cast<uint32_t>(texture->Width),
              static_cast<uint32_t>(texture->Height),
              1,
          },
      .imageUsage = rhi::ImageUsage::sampled | rhi::ImageUsage::transfer_dst,
  });

  rhi::ResourceDescriptor descriptor = g_device.create_sampled_image_descriptor(
      {.image = image,
       .layout = rhi::ImageLayout::read_only,
       .subresource = {
           .aspect = rhi::ImageAspect::color,
       }});

  auto *backend_texture = new Texture{
      .image = std::move(image),
      .descriptor = std::move(descriptor),
  };

  texture->BackendUserData = backend_texture;
  texture->SetTexID(
      static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(backend_texture)));

  // Deliberately don't SetStatus(OK) here.
  // WantCreate falls through to update_texture(), which uploads the pixels.
}

static void update_texture(rhi::CommandBuffer cmd, ImTextureData *texture) {
  assert(texture);
  assert(texture->BackendUserData);

  auto *backend_texture = static_cast<Texture *>(texture->BackendUserData);

  const bool create = texture->Status == ImTextureStatus_WantCreate;
  assert(create || texture->Status == ImTextureStatus_WantUpdates);

  const uint32_t x = create ? 0u : static_cast<uint32_t>(texture->UpdateRect.x);
  const uint32_t y = create ? 0u : static_cast<uint32_t>(texture->UpdateRect.y);
  const uint32_t width = create ? static_cast<uint32_t>(texture->Width)
                                : static_cast<uint32_t>(texture->UpdateRect.w);
  const uint32_t height = create ? static_cast<uint32_t>(texture->Height)
                                 : static_cast<uint32_t>(texture->UpdateRect.h);

  if (width == 0 || height == 0) {
    texture->SetStatus(ImTextureStatus_OK);
    return;
  }

  const rhi::ImageLayout initial_layout =
      create ? rhi::ImageLayout::undefined : rhi::ImageLayout::read_only;

  cmd.transition_image(backend_texture->image, initial_layout,
                       rhi::ImageLayout::transfer_dst);

  cmd.update(
      {
          .image = backend_texture->image,
          .subresource =
              {
                  .aspect = rhi::ImageAspect::color,
                  .mipLevel = 0,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
          .offset =
              ivec3{
                  static_cast<int32_t>(x),
                  static_cast<int32_t>(y),
                  0,
              },
          .extent = uvec3{width, height, 1},
      },
      texture->GetPixelsAt(x, y),

      // The source pointer starts at (x,y), but successive source rows
      // are still separated by the full texture pitch.
      static_cast<uint32_t>(texture->Width), 0, rhi::ImageLayout::transfer_dst);

  cmd.transition_image(backend_texture->image, rhi::ImageLayout::transfer_dst,
                       rhi::ImageLayout::read_only);

  texture->SetStatus(ImTextureStatus_OK);
}

static void destroy_texture(ImTextureData *texture) {
  assert(texture);
  if (auto *backend_texture =
          static_cast<Texture *>(texture->BackendUserData)) {
    delete backend_texture;
    texture->BackendUserData = nullptr;
    texture->SetTexID(ImTextureID_Invalid);
  }

  texture->SetStatus(ImTextureStatus_Destroyed);
}

static void process_texture(rhi::CommandBuffer cmd, ImTextureData *texture) {
  assert(texture);

  switch (texture->Status) {
  case ImTextureStatus_OK:
  case ImTextureStatus_Destroyed:
    return;

  case ImTextureStatus_WantCreate:
    create_texture(texture);
    [[fallthrough]];

  case ImTextureStatus_WantUpdates:
    update_texture(cmd, texture);
    return;

  case ImTextureStatus_WantDestroy:
    destroy_texture(texture);
    return;
  }

  std::unreachable();
}
static void set_sampler_linear(rhi::CommandBuffer cmd) noexcept {
  cmd.push(20, g_linear_sampler);
}

static void set_sampler_nearest(rhi::CommandBuffer cmd) noexcept {
  cmd.push(20, g_nearest_sampler);
}

static void set_default_rendering_state(rhi::CommandBuffer cmd, vec2 viewport,
                                        rhi::SampleCount sampleCount,
                                        vec2 scale, vec2 translate) {
  cmd.set_viewport({
      .position = {0, 0},
      .extent = viewport,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  });

  std::array bindings{
      rhi::VertexBinding{
          .binding = 0,
          .stride = sizeof(ImDrawVert),
          .inputRate = rhi::VertexInputRate::vertex,
      },
  };
  std::array attributes{
      rhi::VertexAttribute{
          .location = 0,
          .binding = 0,
          .format = rhi::Format::rg32_float,
          .offset = offsetof(ImDrawVert, pos),
      },
      rhi::VertexAttribute{
          .location = 1,
          .binding = 0,
          .format = rhi::Format::rg32_float,
          .offset = offsetof(ImDrawVert, uv),
      },
      rhi::VertexAttribute{
          .location = 2,
          .binding = 0,
          .format = rhi::Format::rgba8_unorm,
          .offset = offsetof(ImDrawVert, col),
      },
  };
  cmd.set_vertex_input(bindings, attributes);

  cmd.bind_vertex_buffer(g_vertex_buffer);
  cmd.bind_index_buffer(g_index_buffer, sizeof(ImDrawIdx) == 2
                                            ? rhi::IndexType::uint16
                                            : rhi::IndexType::uint32);

  cmd.set_primitive_topology(rhi::PrimitiveTopology::triangle_list);
  cmd.set_primitive_restart(false);
  cmd.set_rasterizer_discard_enable(false);
  cmd.set_polygon_mode(rhi::PolygonMode::fill);
  cmd.set_cull_mode(rhi::CullMode::none);
  cmd.set_front_face(rhi::FrontFace::counter_clockwise);
  cmd.set_depth_bias_enable(false);

  cmd.set_rasterization_samples(sampleCount);
  cmd.set_sample_mask(sampleCount); // enable all
  cmd.set_alpha_to_coverage_enable(false);
  cmd.set_alpha_to_one_enable(false);
  cmd.set_depth_test_enable(false);
  cmd.set_stencil_test_enable(false);

  cmd.set_color_blend_enable(0, 1, true);

  rhi::BlendEquation blendEq{
      .srcColor = rhi::BlendFactor::src_alpha,
      .dstColor = rhi::BlendFactor::one_minus_src_alpha,
      .colorBlendOp = rhi::BlendOp::add,
      .srcAlpha = rhi::BlendFactor::one,
      .dstAlpha = rhi::BlendFactor::one_minus_src_alpha,
      .alphaBlendOp = rhi::BlendOp::add,
  };
  cmd.set_color_blend_equation(0, &blendEq);
  rhi::ColorComponent rgba = rhi::ColorComponent::rgba;
  cmd.set_color_write_mask(0, &rgba);

  cmd.bind_shader(g_vertex_shader);
  cmd.bind_shader(g_fragment_shader);

  cmd.push(0, &scale, sizeof(scale));
  cmd.push(8, &translate, sizeof(translate));

  set_sampler_linear(cmd);
}

void render(rhi::CommandBuffer cmd, RenderInfo info) noexcept {
  ImGui::Render();
  const ImDrawData *drawData = ImGui::GetDrawData();

  const uvec2 extent{
      uint32_t(drawData->DisplaySize.x * drawData->FramebufferScale.x + 0.5f),
      uint32_t(drawData->DisplaySize.y * drawData->FramebufferScale.y + 0.5f),
  };

  if (!drawData->Valid || extent.x() == 0 || extent.y() == 0) {
    if (info.inital_layout != info.final_layout) {
      cmd.transition_image(info.target.image(), info.inital_layout,
                           info.final_layout);
    }
    return;
  }

  // texture-updates:
  if (drawData->Textures) {
    for (ImTextureData *texture : *drawData->Textures) {
      if (texture->Status != ImTextureStatus_OK) {
        process_texture(cmd, texture);
      }
    }
  }

  const uint64_t vertex_size =
      uint64_t(drawData->TotalVtxCount) * sizeof(ImDrawVert);

  if (g_vertex_capacity < vertex_size) {
    g_vertex_capacity = std::max(vertex_size, g_vertex_capacity * 2);

    g_vertex_buffer = g_device.create_buffer({
        .size = g_vertex_capacity,
        .bufferUsage =
            rhi::BufferUsage::vertex | rhi::BufferUsage::transfer_dst,
        .memoryUsage = rhi::MemoryUsage::automatic,
    });

    free(g_host_vertex_stage);
    g_host_vertex_stage = malloc(g_vertex_capacity);
    assert(g_host_vertex_stage);
  }

  const uint64_t index_size =
      uint64_t(drawData->TotalIdxCount) * sizeof(ImDrawIdx);

  if (g_index_capacity < index_size) {
    g_index_capacity = std::max(index_size, g_index_capacity * 2);

    g_index_buffer = g_device.create_buffer({
        .size = g_index_capacity,
        .bufferUsage = rhi::BufferUsage::index | rhi::BufferUsage::transfer_dst,
        .memoryUsage = rhi::MemoryUsage::automatic,
    });

    free(g_host_index_stage);
    g_host_index_stage = malloc(g_index_capacity);
    assert(g_host_index_stage);
  }

  std::byte *vertex_dst = static_cast<std::byte *>(g_host_vertex_stage);

  std::byte *index_dst = static_cast<std::byte *>(g_host_index_stage);

  for (const ImDrawList *list : drawData->CmdLists) {
    const uint64_t list_vertex_size =
        uint64_t(list->VtxBuffer.Size) * sizeof(ImDrawVert);

    const uint64_t list_index_size =
        uint64_t(list->IdxBuffer.Size) * sizeof(ImDrawIdx);

    memcpy(vertex_dst, list->VtxBuffer.Data, list_vertex_size);
    memcpy(index_dst, list->IdxBuffer.Data, list_index_size);

    vertex_dst += list_vertex_size;
    index_dst += list_index_size;
  }
  if (vertex_size > 0) {
    cmd.update({.buffer = g_vertex_buffer}, g_host_vertex_stage, vertex_size);
  }
  if (index_size > 0) {
    cmd.update({.buffer = g_index_buffer}, g_host_index_stage, index_size);
  }

  // big barrier
  cmd.memory_barrier({
      .srcStage = rhi::PipelineStage::transfer,
      .srcAccess = rhi::Access::transfer_write,

      .dstStage = rhi::PipelineStage::vertex_attribute_input |
                  rhi::PipelineStage::index_input,
      .dstAccess = rhi::Access::vertex_attribute_read | rhi::Access::index_read,
  });

  // begin rendering
  if (info.inital_layout != rhi::ImageLayout::attachment) {
    cmd.transition_image(info.target.image(), info.inital_layout,
                         rhi::ImageLayout::attachment);
  }

  if (!drawData->CmdLists.empty()) {

    rhi::Attachment attachment{
        .view = info.target.view(),
        .layout = rhi::ImageLayout::attachment,
        .loadOp = info.inital_layout == rhi::ImageLayout::undefined
                      ? rhi::AttachmentLoadOp::clear
                      : rhi::AttachmentLoadOp::load,
        .storeOp = rhi::AttachmentStoreOp::store,
    };
    cmd.begin_rendering({.colorAttachments = &attachment});
    vec2 scale{
        2.0f / drawData->DisplaySize.x,
        2.0f / drawData->DisplaySize.y,
    };
    vec2 translate{
        -1.0f - drawData->DisplayPos.x * scale.x(),
        -1.0f - drawData->DisplayPos.y * scale.y(),
    };

    set_default_rendering_state(cmd, extent, info.target.image().samples(),
                                scale, translate);

    int32_t global_vertex_offset = 0;
    uint32_t global_index_offset = 0;

    auto &platform_io = ImGui::GetPlatformIO();

    for (const ImDrawList *list : drawData->CmdLists) {
      for (const ImDrawCmd &draw : list->CmdBuffer) {
        // === Callbacks ====
        if (draw.UserCallback) {
          if (draw.UserCallback == platform_io.DrawCallback_ResetRenderState) {
            set_default_rendering_state(
                cmd, extent, info.target.image().samples(), scale, translate);
          } else if (draw.UserCallback ==
                     platform_io.DrawCallback_SetSamplerLinear) {
            set_sampler_linear(cmd);
          } else if (draw.UserCallback ==
                     platform_io.DrawCallback_SetSamplerNearest) {
            set_sampler_nearest(cmd);
          } else {
            draw.UserCallback(list, &draw);
          }
          continue;
        }

        // === Clipping ====
        const ImVec2 clip_offset = drawData->DisplayPos;
        const ImVec2 clip_scale = drawData->FramebufferScale;
        ImVec2 clip_min{
            (draw.ClipRect.x - clip_offset.x) * clip_scale.x,
            (draw.ClipRect.y - clip_offset.y) * clip_scale.y,
        };
        ImVec2 clip_max{
            (draw.ClipRect.z - clip_offset.x) * clip_scale.x,
            (draw.ClipRect.w - clip_offset.y) * clip_scale.y,
        };
        clip_min.x = std::max(clip_min.x, 0.0f);
        clip_min.y = std::max(clip_min.y, 0.0f);
        clip_max.x = std::min(clip_max.x, float(extent.x()));
        clip_max.y = std::min(clip_max.y, float(extent.y()));
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) {
          continue;
        }
        cmd.set_scissor({
            .offset =
                {
                    static_cast<int32_t>(clip_min.x),
                    static_cast<int32_t>(clip_min.y),
                },
            .extent =
                {
                    static_cast<uint32_t>(clip_max.x - clip_min.x),
                    static_cast<uint32_t>(clip_max.y - clip_min.y),
                },
        });

        // can this be null, how do we check if the draw even has a texture???
        // or do all draws have a texture??
        ImTextureID id = draw.GetTexID();
        assert(id != ImTextureID_Invalid);
        auto *texture = reinterpret_cast<Texture *>(static_cast<uintptr_t>(id));
        assert(texture);

        cmd.push(16,
                 texture->descriptor); // uint32_t in shader as push constants.
        cmd.draw_indexed(draw.ElemCount, 1,
                         global_index_offset + draw.IdxOffset,
                         global_vertex_offset + draw.VtxOffset, 0);
      }
      global_vertex_offset += list->VtxBuffer.Size;
      global_index_offset += list->IdxBuffer.Size;
    }

    // end rendering
    cmd.end_rendering();
  }
  if (info.final_layout != rhi::ImageLayout::attachment) {
    cmd.transition_image(info.target.image(), rhi::ImageLayout::attachment,
                         info.final_layout);
  }
}

void shutdown() noexcept {
  auto &platform_io = ImGui::GetPlatformIO();
  for (ImTextureData *texture : platform_io.Textures) {
    if (texture->BackendUserData) {
      destroy_texture(texture);
    }
  }
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  g_device = {};

  g_vertex_buffer = {};
  g_vertex_capacity = 0;
  free(g_host_vertex_stage);
  g_host_vertex_stage = nullptr;

  g_index_buffer = {};
  g_index_capacity = 0;
  free(g_host_index_stage);
  g_host_index_stage = nullptr;

  g_linear_sampler = {};
  g_nearest_sampler = {};

  g_vertex_shader = {};
  g_fragment_shader = {};
}

} // namespace strobe::imgui
