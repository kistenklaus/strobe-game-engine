#pragma once

#include "imgui.h"
#include "strobe/imgui/renderer/frame.hpp"
#include "strobe/imgui/renderer/samplers.hpp"
#include "strobe/imgui/renderer/shaders.hpp"
#include "strobe/imgui/renderer/texture.hpp"
#include "strobe/rhi/objects/image_view.hpp"

namespace strobe::imgui::renderer {

void draw(rhi::CommandBuffer cmd, rhi::ImageView target,
          rhi::AttachmentLoadOp loadOp, const ImDrawData *drawData,
          rhi::Device &device, Frame &frame, Samplers *samplers,
          Shaders *shaders);

} // namespace strobe::imgui::renderer
