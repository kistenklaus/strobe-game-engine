#pragma once

#include "strobe/gpu/device/shader_stage.hpp"
namespace strobe::gpu {

struct PushConstantRange {
  ShaderStage stage;
  uint32_t offset;
  uint32_t size;
};
} // namespace strobe::gpu
