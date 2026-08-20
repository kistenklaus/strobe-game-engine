#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/gpu/device/command_buffer_state_alloctor.hpp"
#include "strobe/gpu/device/fragment_shader.hpp"
#include "strobe/gpu/device/vertex_shader.hpp"

namespace strobe::gpu {

struct CommandBufferState {
  using allocator = cmd_buf_state_allocator_ref;
  CommandBufferState(const allocator &alloc) noexcept
      : boundVertexShaders(alloc), boundFragmentShaders(alloc) {}

  Vector<VertexShader, allocator> boundVertexShaders;
  Vector<FragmentShader, allocator> boundFragmentShaders;
};

} // namespace strobe::gpu
