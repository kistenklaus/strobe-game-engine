#pragma once

#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/vulkan/shader_object.hpp"
namespace strobe::gpu {

struct ShaderObjectImpl {
  Context context;

  vulkan::ShaderObject shaderObject;
};

}


