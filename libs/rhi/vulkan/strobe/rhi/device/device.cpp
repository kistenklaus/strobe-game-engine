#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/cmd/cmd.hpp"
#include "strobe/rhi/device/device_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/handle_allocators.hpp"
#include "strobe/rhi/shader/shader.hpp"
#include "strobe/rhi/swapchain/swap.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
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

FragmentShader
Device::create_fragment_shader(const FragmentShaderInfo &info) noexcept {
  ZoneScopedN("Device::create_fragment_shader");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return shader::create_fragment(impl->context, info,
                                 &impl->allocs->shaderAlloc);
}

VertexShader
Device::create_vertex_shader(const VertexShaderInfo &info) noexcept {
  ZoneScopedN("Device::create_vertex_shader");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return shader::create_vertex(impl->context, info, &impl->allocs->shaderAlloc);
}

ComputeShader
Device::create_compute_shader(const ComputeShaderInfo &info) noexcept {
  ZoneScopedN("Device::create_compute_shader");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return shader::create_compute(impl->context, info,
                                &impl->allocs->shaderAlloc);
}

Swapchain Device::create_swapchain(const SwapchainInfo &info) noexcept {
  ZoneScopedN("Device::create_swapchain");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);

  // NOTE: only the universal currently supports presentation.
  Vector<uint32_t, strobe::rhi::allocator_ref> queueFamilyIndicies{
      1, impl->allocs->alloc};
  queueFamilyIndicies[0] =
      object_handle_ptr<QueueImpl>(impl->universalQueue)->family();
  return swap::create_swapchain(impl->context, impl->fencePool, impl->semPool,
                                info, std::move(queueFamilyIndicies),
                                &impl->allocs->swapAlloc);
}

Queue Device::get_queue([[maybe_unused]] QueueFlags flags) noexcept {
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return impl->universalQueue;
}

CommandPool Device::create_cmdpool() noexcept {
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return cmd::create_cmd_pool(
      impl->context, impl->staging,
      object_handle_ptr<QueueImpl>(impl->universalQueue)->family(),
      &impl->allocs->cmdAlloc);
}

Buffer Device::create_buffer(const BufferInfo &info,
                             const MemoryLifetime &lifetime) {
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return buf::create_buffer(impl->memory, info, lifetime, &impl->allocs->bufAlloc);
}

} // namespace strobe::rhi
