#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/types/device_info.hpp"

/**
 * \defgroup rhi Rendering Hardware Interface
 * \brief Strobe's low-level explicit graphics API.
 *
 * The RHI owns Vulkan object wrappers, command submission,
 * synchronization, and resource interfaces.
 */
namespace strobe::rhi {

Device create_device(const DeviceInfo &info);

}
