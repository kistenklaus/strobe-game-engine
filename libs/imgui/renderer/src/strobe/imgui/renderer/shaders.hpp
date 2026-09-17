#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"

namespace strobe::imgui::renderer {

struct Shaders {
public:
  static constexpr uint32_t PC_SCALE_OFFSET = 0;
  static constexpr uint32_t PC_SCALE_SIZE = sizeof(vec2);
  static constexpr uint32_t PC_TRANSLATE_OFFSET = PC_SCALE_SIZE;
  static constexpr uint32_t PC_TRANSLATE_SIZE = sizeof(vec2);
  static constexpr uint32_t PC_TEXTURE_INDEX_OFFSET =
      PC_TRANSLATE_OFFSET + PC_TRANSLATE_SIZE;
  static constexpr uint32_t PC_TEXTURE_INDEX_SIZE = sizeof(uint32_t);
  static constexpr uint32_t PC_SAMPLER_INDEX_OFFSET =
      PC_TEXTURE_INDEX_OFFSET + PC_TEXTURE_INDEX_SIZE;
  static constexpr uint32_t PC_SAMPLER_INDEX_SIZE = sizeof(uint32_t);

  explicit Shaders(rhi::Device &device) noexcept;

  void bind(rhi::CommandBuffer &cmd) const noexcept;

private:
  rhi::VertexShader m_vertexShader;
  rhi::FragmentShader m_fragmentShader;
};

} // namespace strobe::imgui
