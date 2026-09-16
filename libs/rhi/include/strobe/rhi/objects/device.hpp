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
#include "strobe/rhi/objects/resource_descriptor.hpp"
#include "strobe/rhi/objects/sampler_descriptor.hpp"
#include "strobe/rhi/objects/sampler_descriptor_array.hpp"
#include "strobe/rhi/objects/swapchain.hpp"
#include "strobe/rhi/objects/tlas.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"
#include "strobe/rhi/types/blas_info.hpp"
#include "strobe/rhi/types/buffer_info.hpp"
#include "strobe/rhi/types/compute_shader_info.hpp"
#include "strobe/rhi/types/fragment_shader_info.hpp"
#include "strobe/rhi/types/image_info.hpp"
#include "strobe/rhi/types/image_view_info.hpp"
#include "strobe/rhi/types/memory_lifetime.hpp"
#include "strobe/rhi/types/queue_flags.hpp"
#include "strobe/rhi/types/resource_descriptor_info.hpp"
#include "strobe/rhi/types/sampler_descriptor_info.hpp"
#include "strobe/rhi/types/swapchain_info.hpp"
#include "strobe/rhi/types/tlas_info.hpp"
#include "strobe/rhi/types/vertex_shader_info.hpp"
#include <limits>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Device resource factory.
 * Defined in header <strobe/rhi/rhi.hpp>
 *
 * \code{.cpp}
 * class Device : public Object<Device>;
 * \endcode
 *
 * Creates device-owned resources and provides access to queues and asynchronous
 * transfer operations.
 */
class Device : public Object<Device> {
  friend class Object<Device>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;

  /**
   * \brief Creates fragment shader.
   * \code{.cpp}
   * FragmentShader create_fragment_shader(const FragmentShaderInfo& info) noexcept;
   * \endcode
   *
   * Creates a fragment shader from \p info.
   *
   * \param info Fragment shader creation information.
   *
   * \return Created fragment shader.
   *
   * \attention 1. \p info must describe a valid fragment shader.
   */
  FragmentShader
  create_fragment_shader(const FragmentShaderInfo &info) noexcept;

  /**
   * \brief Creates vertex shader.
   * \code{.cpp}
   * VertexShader create_vertex_shader(const VertexShaderInfo& info) noexcept;
   * \endcode
   *
   * Creates a vertex shader from \p info.
   *
   * \param info Vertex shader creation information.
   *
   * \return Created vertex shader.
   *
   * \attention 1. \p info must describe a valid vertex shader.
   */
  VertexShader create_vertex_shader(const VertexShaderInfo &info) noexcept;

  /**
   * \brief Creates compute shader.
   * \code{.cpp}
   * ComputeShader create_compute_shader(const ComputeShaderInfo& info) noexcept;
   * \endcode
   *
   * Creates a compute shader from \p info.
   *
   * \param info Compute shader creation information.
   *
   * \return Created compute shader.
   *
   * \attention 1. \p info must describe a valid compute shader.
   */
  ComputeShader create_compute_shader(const ComputeShaderInfo &info) noexcept;

  /**
   * \brief Creates swapchain.
   * \code{.cpp}
   * Swapchain create_swapchain(const SwapchainInfo& info) noexcept;
   * \endcode
   *
   * Creates a presentation swapchain from \p info.
   *
   * \param info Swapchain creation information.
   *
   * \return Created swapchain.
   *
   * \attention 1. \p info must reference a valid presentation surface.
   */
  Swapchain create_swapchain(const SwapchainInfo &info) noexcept;

  /**
   * \brief Gets device queue.
   * \code{.cpp}
   * Queue get_queue(QueueFlags flags = QueueFlags::graphics | QueueFlags::compute | QueueFlags::transfer) noexcept;
   * \endcode
   *
   * Returns a queue supporting the requested capabilities.
   *
   * \param flags Required queue capabilities.
   *
   * \return Compatible device queue.
   *
   * \attention 1. A queue supporting all capabilities in \p flags must exist.
   */
  Queue get_queue(QueueFlags flags = QueueFlags::graphics |
                                     QueueFlags::compute |
                                     QueueFlags::transfer) noexcept;

  /**
   * \brief Creates command pool.
   * \code{.cpp}
   * CommandPool create_cmdpool() noexcept;
   * \endcode
   *
   * Creates a command pool associated with this device.
   *
   * \return Created command pool.
   */
  CommandPool create_cmdpool() noexcept;

  /**
   * \brief Creates buffer.
   * \code{.cpp}
   * Buffer create_buffer(const BufferInfo& info, const MemoryLifetime& lifetime = {}) noexcept;
   * \endcode
   *
   * Creates a buffer using the requested memory lifetime.
   *
   * \param info Buffer creation information.
   * \param lifetime Requested memory lifetime.
   *
   * \return Created buffer.
   *
   * \attention 1. \p info must describe a valid buffer.
   * \attention 2. \p lifetime must remain valid for the intended use of the
   * allocation.
   */
  Buffer create_buffer(const BufferInfo &info,
                       const MemoryLifetime &lifetime = {}) noexcept;

  /**
   * \brief Creates image.
   * \code{.cpp}
   * Image create_image(const ImageInfo& info, const MemoryLifetime = {}) noexcept;
   * \endcode
   *
   * Creates an image using the requested memory lifetime.
   *
   * \param info Image creation information.
   * \param lifetime Requested memory lifetime.
   *
   * \return Created image.
   *
   * \attention 1. \p info must describe a valid image.
   * \attention 2. \p lifetime must remain valid for the intended use of the
   * allocation.
   */
  Image create_image(const ImageInfo &info,
                     const MemoryLifetime &lifetime = {}) noexcept;

  /**
   * \brief Creates image view.
   * \code{.cpp}
   * ImageView create_image_view(const Image& image, const ImageViewInfo& info) noexcept;
   * \endcode
   *
   * Creates a view into \p image.
   *
   * \param image Source image.
   * \param info Image view creation information.
   *
   * \return Created image view.
   *
   * \attention 1. \p image must be valid.
   * \attention 2. \p info must describe a valid view of \p image.
   */
  ImageView create_image_view(const Image &image,
                              const ImageViewInfo &info) noexcept;

  /**
   * \brief Creates BLAS.
   * \code{.cpp}
   * Blas create_blas(const BlasInfo& info, const MemoryLifetime& = {}) noexcept;
   * \endcode
   *
   * Creates a bottom-level acceleration structure.
   *
   * \param info BLAS creation information.
   * \param lifetime Requested memory lifetime.
   *
   * \return Created BLAS.
   *
   * \attention 1. \p info must describe a valid bottom-level acceleration
   * structure.
   * \attention 2. \p lifetime must remain valid for the intended use of the
   * allocation.
   */
  Blas create_blas(const BlasInfo &info,
                   const MemoryLifetime &lifetime = {}) noexcept;

  /**
   * \brief Creates TLAS.
   * \code{.cpp}
   * Tlas create_tlas(const TlasInfo& info, const MemoryLifetime& lifetime = {}) noexcept;
   * \endcode
   *
   * Creates a top-level acceleration structure.
   *
   * \param info TLAS creation information.
   * \param lifetime Requested memory lifetime.
   *
   * \return Created TLAS.
   *
   * \attention 1. \p info must describe a valid top-level acceleration
   * structure.
   * \attention 2. \p lifetime must remain valid for the intended use of the
   * allocation.
   */
  Tlas create_tlas(const TlasInfo &info,
                   const MemoryLifetime &lifetime = {}) noexcept;

  /**
   * \brief Copies buffer asynchronously.
   * \code{.cpp}
   * Timepoint async_copy(BufferOffset dst, BufferOffset src, uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;
   * \endcode
   *
   * Schedules a device-side copy from \p src to \p dst.
   *
   * \param dst Destination buffer and offset.
   * \param src Source buffer and offset.
   * \param size Number of bytes to copy.
   *
   * \return Timepoint at which the copy is complete.
   *
   * \attention 1. Source and destination ranges must be valid.
   * \attention 2. Source and destination ranges must not overlap when they
   * reference the same underlying buffer.
   * \attention 3. Source and destination buffers must support the required
   * transfer usage.
   * \attention 4. Source and destination resources must remain valid until the
   * returned Timepoint completes.
   */
  Timepoint
  async_copy(BufferOffset dst, BufferOffset src,
             uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;

  /**
   * \brief Uploads buffer asynchronously.
   * \code{.cpp}
   * Timepoint async_upload(BufferOffset dst, void* src, uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;
   * \endcode
   *
   * Schedules a host-to-device upload into \p dst.
   *
   * \param dst Destination buffer and offset.
   * \param src Source data.
   * \param size Number of bytes to upload.
   *
   * \return Timepoint at which the upload is complete.
   *
   * \attention 1. The destination range must be valid.
   * \attention 2. The destination buffer must support the required transfer
   * usage.
   * \attention 3. \p src must reference at least \p size bytes of valid data.
   */
  Timepoint
  async_upload(BufferOffset dst, void *src,
               uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;

  /**
   * \brief Creates resource descriptor.
   * \code{.cpp}
   * ResourceDescriptor create_resource_descriptor(const ResourceDescriptorInfo& info) noexcept;                       (1)
   * ResourceDescriptor create_storage_buffer_descriptor(const StorageBufferDescriptorInfo& info) noexcept;            (2)
   * ResourceDescriptor create_storage_texel_buffer_descriptor(const StorageTexelBufferDescriptorInfo& info) noexcept; (3)
   * ResourceDescriptor create_uniform_buffer_descriptor(const UniformBufferDescriptorInfo& info) noexcept;            (4)
   * ResourceDescriptor create_uniform_texel_buffer_descriptor(cont UniformTexelBufferDescriptorInfo& info) noexcept;  (5)
   * ResourceDescriptor create_sampled_image_descriptor(const SampledImageDescriptorInfo& info) noexcept;              (6)
   * ResourceDescriptor create_storage_image_descriptor(const StorageImageDescriptorInfo& info) noexcept;              (7)
   * \endcode
   *
   * Creates a descriptor from \p info.
   *
   * \param info Resource descriptor creation information.
   *
   * \return Created resource descriptor.
   *
   * \attention 1. \p info must describe a valid resource descriptor.
   * \attention 2. Referenced resources must remain valid while the descriptor
   * may be used.
   */
  ResourceDescriptor
  create_resource_descriptor(const ResourceDescriptorInfo &info) noexcept;
  inline ResourceDescriptor create_storage_buffer_descriptor(
      const StorageBufferDescriptorInfo &info) noexcept {
    return create_resource_descriptor(info);
  }
  inline ResourceDescriptor create_storage_texel_buffer_descriptor(
      const StorageTexelBufferDescriptorInfo &info) noexcept {
    return create_resource_descriptor(info);
  }
  inline ResourceDescriptor create_uniform_buffer_descriptor(
      const UniformBufferDescriptorInfo &info) noexcept {
    return create_resource_descriptor(info);
  }
  inline ResourceDescriptor create_uniform_texel_buffer_descriptor(
      const UniformTexelBufferDescriptorInfo &info) noexcept {
    return create_resource_descriptor(info);
  }
  inline ResourceDescriptor create_sampled_image_descriptor(
      const SampledImageDescriptorInfo &info) noexcept {
    return create_resource_descriptor(info);
  }
  inline ResourceDescriptor create_storage_image_descriptor(
      const StorageImageDescriptorInfo &info) noexcept {
    return create_resource_descriptor(info);
  }

  ResourceDescriptorArray create_resource_descriptors(
      span<const ResourceDescriptorInfo> infos) noexcept;

  SamplerDescriptor
  create_sampler_descriptor(const SamplerDescriptorInfo &info) noexcept;

  SamplerDescriptorArray
  create_sampler_descriptors(span<const SamplerDescriptorInfo> infos) noexcept;
};

} // namespace strobe::rhi
