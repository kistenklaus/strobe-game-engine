#pragma once

#include "strobe/core/memory/AllocatorReference.hpp"
#include "strobe/core/memory/inplace_monotonic_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/vulkan/shader_object.hpp"

namespace strobe::rhi::details {

VertexShader create_vertex_shader(Context context,
                                  const VertexShaderInfo &info) {

  using scratch_allocator =
      InplaceMonotonicResource<strobe::rhi::scratch_allocator, 1 << 10>;
  using scratch_allocator_ref = AllocatorReference<scratch_allocator>;

  scratch_allocator scratch{};

  vulkan::create_shader_object(context.ctx(), {
                                                  .pushConstantRange = {},
                                              });
}

vulkan::ShaderObject create_shader_object() {

}

} // namespace strobe::rhi::details
