#pragma once

#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/vulkan/shader_object.hpp"
namespace strobe::gpu {

struct ShaderObjectImpl {

  ShaderObjectImpl(Context context, vulkan::ShaderObject shader)
      : context(std::move(context)), shader(shader) {}
  ~ShaderObjectImpl() noexcept {
    vulkan::destroy_shader_object(context.get(), shader);
  }

  const Context context;
  const vulkan::ShaderObject shader;
};

} // namespace strobe::gpu
