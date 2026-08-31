#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/device/device_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

Device::Device(const Device &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<DeviceImpl>(m_handle);
  }
}

Device::Device(Device &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

Device &Device::operator=(const Device &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<DeviceImpl>(o.m_handle);
  }
  unpin_void_handle<DeviceImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Device &Device::operator=(Device &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<DeviceImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Device::~Device() noexcept { unpin_void_handle<DeviceImpl>(m_handle); }

} // namespace strobe::rhi
