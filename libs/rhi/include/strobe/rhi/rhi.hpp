#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/types/device_info.hpp"

namespace strobe::rhi {

Device create_device(const DeviceInfo &info);

}
