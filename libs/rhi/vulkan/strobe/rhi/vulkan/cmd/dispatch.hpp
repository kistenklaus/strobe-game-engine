#pragma once

#include "strobe/rhi/vulkan/command_buffer.hpp"

namespace strobe::rhi::vulkan {

void cmd_dispatch(CommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY,
                  uint32_t groupCountZ) noexcept;
}
