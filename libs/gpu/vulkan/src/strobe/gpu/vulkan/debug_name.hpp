#pragma once

#include "strobe/gpu/vulkan/binary_semaphore.hpp"
#include "strobe/gpu/vulkan/buffer.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"
#include "strobe/gpu/vulkan/command_pool.hpp"
#include "strobe/gpu/vulkan/fence.hpp"
#include "strobe/gpu/vulkan/image.hpp"
#include "strobe/gpu/vulkan/shader_object.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

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

} // namespace strobe::gpu::vulkan
