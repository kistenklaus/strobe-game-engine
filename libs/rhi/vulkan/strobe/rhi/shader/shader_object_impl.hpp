#pragma once

#include "strobe/rhi/device/context.hpp"
#include "strobe/rhi/vulkan/shader_object.hpp"

namespace strobe::rhi {

struct ShaderObjectImpl {

  ShaderObjectImpl(Context context, vulkan::ShaderObject shader)
      : context(std::move(context)), shader(shader) {}
  ~ShaderObjectImpl() noexcept {
    vulkan::destroy_shader_object(context.ctx(), shader);
  }

  const Context context;
  const vulkan::ShaderObject shader;
};

} // namespace strobe::rhi
