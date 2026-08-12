#pragma once

#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/device_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/vulkan/surface.hpp"
namespace strobe::gpu {

struct SurfaceImpl {

  SurfaceImpl(Device device, vulkan::Surface surface) noexcept
      : device(std::move(device)), surface(surface) {}

  ~SurfaceImpl() noexcept {
    auto *device_impl = void_handle_ptr<DeviceImpl>(device.m_handle);
    vulkan::destroy_surface(&device_impl->context, surface);
  }

  Device device;
  vulkan::Surface surface;
};

} // namespace strobe::gpu
