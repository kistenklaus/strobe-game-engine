#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/shader_stage.hpp"

namespace strobe::rhi {

struct VertexShaderInfo {
  span<const uint32_t> spirv = {};
  ShaderStage nextStage = ShaderStage::fragment;
};

/**
 * \ingroup rhi
 * \brief todo
 */
struct VertexShader : public Object<VertexShader> {
  friend class Object<VertexShader>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;
};

} // namespace strobe::rhi
