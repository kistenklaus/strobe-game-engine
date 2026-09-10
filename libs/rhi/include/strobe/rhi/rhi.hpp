#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/types/device_info.hpp"

/**
 * \defgroup rhi Render Hardware Interface
 * \brief Low-level Vulkan-oriented graphics and compute abstraction.
 *
 * The Render Hardware Interface provides the low-level GPU API used by Strobe.
 * It wraps Vulkan objects while exposing explicit synchronization, memory
 * lifetime management, queues, command buffers, and resource descriptors.
 *
 * \section rhi_design Design goals
 *
 * The RHI is designed to:
 *
 * - Keep Vulkan overhead visible and predictable.
 * - Make resource lifetime and synchronization explicit.
 * - Support asynchronous transfers and multiple queues.
 * - Provide a foundation for render graphs and higher-level renderers.
 *
 * \section rhi_components Main components
 *
 * The RHI is organized around:
 *
 * - \ref strobe::rhi::Device
 * - \ref strobe::rhi::Queue
 * - \ref strobe::rhi::CommandBuffer
 * - \ref strobe::rhi::Buffer
 * - \ref strobe::rhi::Image
 *
 * \section rhi_example Example
 *
 * \code{.cpp}
 * auto buffer = device.create_buffer(...);
 * auto command_buffer = queue.create_command_buffer();
 * \endcode
 */
namespace strobe::rhi {

Device create_device(const DeviceInfo &info);

}
