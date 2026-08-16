#pragma once

#include "strobe/gpu/device/command_buffer_state_alloctor.hpp"

namespace strobe::gpu {

struct CommandBufferState {
  using allocator = cmd_buf_state_allocator_ref;
  CommandBufferState([[maybe_unused]] const allocator &alloc) noexcept {}

  // TODO: bound resource refs. (must be default initalizable)
};

} // namespace strobe::gpu
