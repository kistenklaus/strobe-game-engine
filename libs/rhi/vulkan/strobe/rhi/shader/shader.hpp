#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/shader/shader_object_impl.hpp"

namespace strobe::rhi::shader {

struct handle_allocators {
  explicit handle_allocators(strobe::rhi::allocator_ref alloc) noexcept
      : shaderAlloc(alloc) {}
  handle_allocator<ShaderObjectImpl> shaderAlloc;
};

VertexShader create_vertex(Context context, const VertexShaderInfo &info,
                           handle_allocators *alloc);

FragmentShader create_fragment(Context context, const FragmentShaderInfo &info,
                               handle_allocators *alloc);

ComputeShader create_compute(Context context, const ComputeShaderInfo &info,
                             handle_allocators *alloc);

} // namespace strobe::rhi::shader
