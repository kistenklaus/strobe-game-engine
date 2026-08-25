#pragma once

#include "strobe/rhi/vulkan/binary_semaphore.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/command_buffer.hpp"
#include "strobe/rhi/vulkan/command_pool.hpp"
#include "strobe/rhi/vulkan/fence.hpp"
#include "strobe/rhi/vulkan/image.hpp"
#include "strobe/rhi/vulkan/shader_object.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

void set_debug_name(Context *context, vulkan::BinarySemaphore sem,
                    const char *name);

void set_debug_name(Context *context, vulkan::TimelineSemaphore sem,
                    const char *name);

void set_debug_name(Context *context, vulkan::Buffer buf, const char *name);

void set_debug_name(Context *context, vulkan::Image img, const char *name);

void set_debug_name(Context *context, vulkan::CommandBuffer cmd,
                    const char *name);

void set_debug_name(Context *context, vulkan::CommandPool pool,
                    const char *name);

void set_debug_name(Context *context, vulkan::Fence fence, const char *name);

void set_debug_name(Context *context, vulkan::ShaderObject, const char *name);

} // namespace strobe::rhi::vulkan
