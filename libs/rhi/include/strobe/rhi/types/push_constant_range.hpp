#pragma once

#include "strobe/rhi/types/shader_stage.hpp"

namespace strobe::rhi {

struct PushConstantRange {
  ShaderStage stage;
  uint32_t offset;
  uint32_t size;
};
} // namespace strobe::rhi
