#pragma once

#include "strobe/rhi/types/image_layout.hpp"
#include "strobe/rhi/types/image_subresource_range.hpp"
#include "strobe/rhi/types/memory_barrier.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/image.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

void cmd_pipeline_barrier(
    CommandBuffer cmd, span<const VkMemoryBarrier2> memoryBarriers,
    span<const VkBufferMemoryBarrier2> bufferMemoryBarriers,
    span<const VkImageMemoryBarrier2> imageMemoryBarriers,
    VkDependencyFlags dependencyFlags) noexcept;

void cmd_memory_barrier(CommandBuffer cmd,
                        span<const MemoryBarrier> memoryBarriers,
                        VkDependencyFlags dependencyFlags) noexcept;

void cmd_memory_barrier(CommandBuffer cmd, MemoryBarrier memoryBarrier,
                        VkDependencyFlags dependencyFlags) noexcept;

// likely not really optimal, and way broader than required,
// but only really use more fine grained barriers, if there is a
// actual performance need for this and it can be seen in profilers.
void cmd_transition_image(CommandBuffer cmd, Image image,
                          ImageSubresourceRange subresource, ImageLayout src,
                          ImageLayout dst) noexcept;

} // namespace strobe::rhi::vulkan
