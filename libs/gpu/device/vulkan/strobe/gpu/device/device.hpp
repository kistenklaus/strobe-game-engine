#pragma once

#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/device_create_info.hpp"
#include "strobe/gpu/device/device_info.hpp"

namespace strobe::gpu {

class Device {
public:
  Device(const DeviceCreateInfo &createInfo);
  Device(const Device &) noexcept;
  Device(Device &&) noexcept;
  Device &operator=(const Device &) noexcept;
  Device &operator=(Device &&) noexcept;
  ~Device() noexcept;

  const DeviceInfo &info() const noexcept;

private:
  void *m_handle;
};

} // namespace strobe::gpu
