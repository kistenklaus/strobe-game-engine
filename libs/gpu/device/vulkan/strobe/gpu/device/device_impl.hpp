#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"
#include "strobe/gpu/device/device_create_info.hpp"

namespace strobe::gpu {

struct DeviceImpl {

  DeviceImpl(const DeviceCreateInfo& createInfo) : context(vulkan::ContextCreateInfo {
      }){

  }

  vulkan::Context context;
};

}
