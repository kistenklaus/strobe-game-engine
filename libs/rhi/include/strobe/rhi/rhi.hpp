#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/types/device_info.hpp"

/**
 * \defgroup rhi Api Reference
 */
namespace strobe::rhi {

Device create_device(const DeviceInfo &info);

}
