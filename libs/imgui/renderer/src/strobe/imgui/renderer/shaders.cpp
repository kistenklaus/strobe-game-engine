#include "strobe/imgui/renderer/shaders.hpp"

#include "strobe/imgui/fragment.slang.spv.hpp"
#include "strobe/imgui/vertex.slang.spv.hpp"

namespace strobe::imgui::renderer {

Shaders::Shaders(rhi::Device &device) noexcept
    : m_vertexShader(device.create_vertex_shader(
          {.spirv = strobe_imgui_vertex_slang_spv,
           .nextStage = rhi::ShaderStage::fragment})),
      m_fragmentShader(device.create_fragment_shader({
          .spirv = strobe_imgui_fragment_slang_spv,
      })) {}

void Shaders::bind(rhi::CommandBuffer &cmd) const noexcept {
  cmd.bind_shader(m_vertexShader);
  cmd.bind_shader(m_fragmentShader);
}

} // namespace strobe::imgui::renderer
