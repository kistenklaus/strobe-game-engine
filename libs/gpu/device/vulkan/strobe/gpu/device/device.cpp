#include "strobe/gpu/device/device.hpp"
#include "strobe/core/memory/smart_pointers/SharedBlock.hpp"
#include "strobe/gpu/device/allocator.hpp"
#include "strobe/gpu/device/device_impl.hpp"
#include "strobe/gpu/device/handle.hpp"

namespace strobe::gpu {

Device::Device(const DeviceCreateInfo &createInfo)
    : m_handle(make_void_handle<DeviceImpl>(createInfo)) {}

Device::Device(const Device &o) noexcept : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<DeviceImpl>(m_handle);
  }
}

Device::Device(Device &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

Device &Device::operator=(const Device &o) noexcept {
  if (this == &o) {
    return *this;
  }
  // Pin first in case both objects refer to the same control block.
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

const DeviceInfo &Device::info() const noexcept {
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
}

} // namespace strobe::gpu
