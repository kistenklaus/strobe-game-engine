#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/bvh/bvh.hpp"
#include "strobe/rhi/cmd/cmd.hpp"
#include "strobe/rhi/device/device_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/handle_allocators.hpp"
#include "strobe/rhi/img/img.hpp"
#include "strobe/rhi/shader/shader.hpp"
#include "strobe/rhi/swapchain/swap.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include <tracy/Tracy.hpp>
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
  ZoneScopedN("Device::create_cmdpool");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return cmd::create_cmd_pool(
      impl->context, impl->staging,
      object_handle_ptr<QueueImpl>(impl->universalQueue)->family(),
      &impl->allocs->cmdAlloc);
}

Buffer Device::create_buffer(const BufferInfo &info,
                             const MemoryLifetime &lifetime) noexcept {
  ZoneScopedN("Device::create_buffer");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return buf::create_buffer(impl->memory, info, lifetime,
                            &impl->allocs->bufAlloc);
}
Image Device::create_image(const ImageInfo &info,
                           const MemoryLifetime &lifetime) noexcept {
  ZoneScopedN("Device::create_image");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return img::create_image(impl->memory, info, lifetime,
                           &impl->allocs->imgAlloc);
}

ImageView Device::create_image_view(const Image &image,
                                    const ImageViewInfo &info) noexcept {
  ZoneScopedN("Device::create_image_view");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return img::create_image_view(image, info, &impl->allocs->imgAlloc);
}

Blas Device::create_blas(const BlasInfo &info,
                         const MemoryLifetime &lifetime) noexcept {
  ZoneScopedN("Device::create_blas");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return bvh::create_blas(impl->memory, impl->scratch, info, lifetime,
                          &impl->allocs->bvhAlloc);
}

Tlas Device::create_tlas(const TlasInfo &info,
                         const MemoryLifetime &lifetime) noexcept {
  ZoneScopedN("Device::create_tlas");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return bvh::create_tlas(impl->memory, impl->scratch, info, lifetime,
                          &impl->allocs->bvhAlloc);
}

Timepoint Device::async_copy(BufferOffset dst, BufferOffset src,
                             uint64_t size) noexcept {
  ZoneScopedN("Device::async_copy");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return impl->dma.async_copy(dst, src, size);
}

Timepoint Device::async_upload(BufferOffset dst, void *src,
                               uint64_t size) noexcept {
  ZoneScopedN("Device::async_upload");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return impl->dma.async_upload(dst, src, size);
}

BufferDescriptor
Device::create_buffer_descriptor(const BufferDescriptorInfo &info) noexcept {
  ZoneScopedN("Device::create_buffer_descriptor");
  auto *impl = void_handle_ptr<DeviceImpl>(m_handle);
  return impl->heapctrl.create_buffer_descriptor(info);
}

} // namespace strobe::rhi
