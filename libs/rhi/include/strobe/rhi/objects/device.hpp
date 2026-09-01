#pragma once

#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/command_pool.hpp"
#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/objects/tlas.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/types/blas_info.hpp"
#include "strobe/rhi/types/buffer_info.hpp"
#include "strobe/rhi/types/image_info.hpp"
#include "strobe/rhi/types/image_view_info.hpp"
#include "strobe/rhi/types/memory_lifetime.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include "strobe/rhi/types/swapchain_info.hpp"
#include "strobe/rhi/types/tlas_info.hpp"

namespace strobe::rhi {

class Device : Object<Device> {
public:
  explicit Device(void *handle) noexcept : Object(handle) {}
  Device() noexcept : Object(nullptr) {}
  Device(const Device &) noexcept;
  Device(Device &&) noexcept;
  Device &operator=(const Device &) noexcept;
  Device &operator=(Device &&) noexcept;
  ~Device() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const Device &lhs, const Device &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const Device &lhs, const Device &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  FragmentShader
  create_fragment_shader(const FragmentShaderInfo &info) noexcept;

  VertexShader create_vertex_shader(const VertexShaderInfo &info) noexcept;

  ComputeShader create_compute_shader(const ComputeShaderInfo &info) noexcept;

  Swapchain create_swapchain(const SwapchainInfo &info) noexcept;

  Queue get_queue(QueueFlags flags = QueueFlags::graphics |
                                     QueueFlags::compute |
                                     QueueFlags::transfer) noexcept;
  CommandPool create_cmdpool() noexcept;

  Buffer create_buffer(const BufferInfo &info,
                       const MemoryLifetime &lifetime = {}) noexcept;

  Image create_image(const ImageInfo &info,
                     const MemoryLifetime &lifetime = {}) noexcept;

  ImageView create_image_view(const Image &image,
                              const ImageViewInfo &info) noexcept;

  Blas create_blas(const BlasInfo &info,
                   const MemoryLifetime &lifetime = {}) noexcept;

  Tlas create_tlas(const TlasInfo &info,
                   const MemoryLifetime &lifetime = {}) noexcept;
};

} // namespace strobe::rhi
