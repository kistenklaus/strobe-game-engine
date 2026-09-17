#include "strobe/imgui/imgui.hpp"
#include "strobe/imgui/imgui_context.hpp"
#include "strobe/imgui/imgui_texture.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>

namespace strobe::imgui {

void begin_frame() noexcept {
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                               ImGuiDockNodeFlags_None);
}
bool should_close() noexcept {}

static void reset_rendering_state(rhi::CommandBuffer cmd, uvec2 viewport,
                                  const ImDrawData *drawData, Frame &frame,
                                  rhi::SampleCount sampleCount) {
  vec2 scale{
      2.0f / drawData->DisplaySize.x,
      2.0f / drawData->DisplaySize.y,
  };
  vec2 translate{
      -1.0f - drawData->DisplayPos.x * scale.x(),
      -1.0f - drawData->DisplayPos.y * scale.y(),
  };

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

  cmd.bind_vertex_buffer(frame.vertex_buffer);
  cmd.bind_index_buffer(frame.index_buffer, sizeof(ImDrawIdx) == 2
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

  g_context->shaders.bind(cmd);

  cmd.push(Shaders::PC_SCALE_OFFSET, &scale, sizeof(scale));
  cmd.push(Shaders::PC_TRANSLATE_OFFSET, &translate, sizeof(translate));

  cmd.push(Shaders::PC_SAMPLER_INDEX_OFFSET,
           g_context->samplers.linearSampler());
}

void render(rhi::CommandBuffer cmd, RenderInfo info) noexcept {
  ImGui::Render();
  const ImDrawData *drawData = ImGui::GetDrawData();
  const uint32_t frameIndex = g_context->frameIndex;

  auto &frame = g_context->frames[frameIndex];

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
  update_textures(g_context->device, cmd, drawData);

  const uint64_t vertex_size =
      uint64_t(drawData->TotalVtxCount) * sizeof(ImDrawVert);
  frame.ensure_vertex_capacity(g_context->device, vertex_size);
  const uint64_t index_size =
      uint64_t(drawData->TotalIdxCount) * sizeof(ImDrawIdx);
  frame.ensure_index_capacity(g_context->device, index_size);

  std::byte *vertex_dst = static_cast<std::byte *>(frame.host_vertex_stage);
  std::byte *index_dst = static_cast<std::byte *>(frame.host_index_stage);
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
    cmd.update({.buffer = frame.vertex_buffer}, frame.host_vertex_stage,
               vertex_size);
  }
  if (index_size > 0) {
    cmd.update({.buffer = frame.index_buffer}, frame.host_index_stage,
               index_size);
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
    reset_rendering_state(cmd, extent, drawData, frame,
                          info.target.image().samples());

    int32_t vertex_offset = 0;
    uint32_t index_offset = 0;
    const auto &platform_io = ImGui::GetPlatformIO();
    for (const ImDrawList *list : drawData->CmdLists) {
      for (const ImDrawCmd &draw : list->CmdBuffer) {
        // === Callbacks ====
        if (draw.UserCallback) {
          if (draw.UserCallback == platform_io.DrawCallback_ResetRenderState) {
            reset_rendering_state(cmd, extent, drawData, frame,
                                  info.target.image().samples());
          } else if (draw.UserCallback ==
                     platform_io.DrawCallback_SetSamplerLinear) {
            cmd.push(Shaders::PC_SAMPLER_INDEX_OFFSET,
                     g_context->samplers.linearSampler());
          } else if (draw.UserCallback ==
                     platform_io.DrawCallback_SetSamplerNearest) {
            cmd.push(Shaders::PC_SAMPLER_INDEX_OFFSET,
                     g_context->samplers.nearestSampler());
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
        const Texture *tex = Texture::from_id(draw.GetTexID());
        cmd.push(Shaders::PC_TEXTURE_INDEX_OFFSET, tex->descriptor());
        cmd.draw_indexed(draw.ElemCount, 1, index_offset + draw.IdxOffset,
                         vertex_offset + draw.VtxOffset, 0);
      }
      vertex_offset += list->VtxBuffer.Size;
      index_offset += list->IdxBuffer.Size;
    }
    cmd.end_rendering();
  }

  if (info.final_layout != rhi::ImageLayout::attachment) {
    cmd.transition_image(info.target.image(), rhi::ImageLayout::attachment,
                         info.final_layout);
  }

  g_context->frameIndex = (frameIndex + 1);
  if (g_context->frameIndex >= g_context->frames.size()) {
    g_context->frameIndex -= g_context->frames.size();
  }
}

} // namespace strobe::imgui
