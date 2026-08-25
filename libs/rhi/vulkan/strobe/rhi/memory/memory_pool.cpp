#include "strobe/rhi/objects/memory_pool.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/blas_impl.hpp"
#include "strobe/rhi/memory/buffer_handle_alloc.hpp"
#include "strobe/rhi/memory/buffer_impl.hpp"
#include "strobe/rhi/memory/image_impl.hpp"
#include "strobe/rhi/memory/memory_allocation_handle_alloc.hpp"
#include "strobe/rhi/memory/memory_allocation_impl.hpp"
#include "strobe/rhi/memory/memory_pool_impl.hpp"
#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/utils/build_flags_utils.hpp"
#include "strobe/rhi/utils/geometry_flags_utils.hpp"
#include "strobe/rhi/utils/index_type_utils.hpp"
#include "strobe/rhi/vulkan/acceleration_structure.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <tracy/Tracy.hpp>
#include <utility>
#include <variant>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

MemoryPool::MemoryPool(const MemoryPool &o) noexcept : m_handle(o.m_handle) {
  if (m_handle) {
    pin_void_handle<MemoryPoolImpl>(m_handle);
  }
}

MemoryPool::MemoryPool(MemoryPool &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

MemoryPool &MemoryPool::operator=(const MemoryPool &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<MemoryPoolImpl>(o.m_handle);
  }
  unpin_void_handle<MemoryPoolImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

MemoryPool &MemoryPool::operator=(MemoryPool &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<MemoryPoolImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

MemoryPool::~MemoryPool() noexcept {
  unpin_void_handle<MemoryPoolImpl>(m_handle);
}

Buffer MemoryPool::create_buffer(const BufferInfo &info,
                                 const MemoryLifetime &lifetime) const {
  ZoneScopedN("MemoryPool::create_buffer");
  auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);

  auto [binding, buffer, internals] = impl->create_buffer(info, lifetime);
  MemoryAllocation memoryAllocation{alloc_void_handle<
      MemoryAllocationImpl, memory_allocation_handle_allocator_ref>(
      impl->get_handle_alloc(), *this, binding, info.memoryUsage, internals)};
  VkDeviceAddress address = 0;
  if (binding) {
    address = vulkan::get_buffer_device_address(impl->context.ctx(), buffer);
  }
  return Buffer{alloc_void_handle<BufferImpl, buffer_handle_alloc_ref>(
      impl->get_buffer_handle_alloc(), impl->context,
      std::move(memoryAllocation), buffer, info.size, address)};
}

Image MemoryPool::create_image(const ImageCreateInfo &info,
                               const MemoryLifetime &lifetime) {
  ZoneScopedN("MemoryPool::create_image");
  auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);

  auto [binding, image, internals] = impl->create_image(info, lifetime);
  MemoryAllocation memoryAllocation{alloc_void_handle<
      MemoryAllocationImpl, memory_allocation_handle_allocator_ref>(
      impl->get_handle_alloc(), *this, binding, info.memoryUsage, internals)};

  return Image{alloc_void_handle<ImageImpl, image_handle_alloc_ref>(
      impl->get_image_handle_alloc(), impl->context,
      std::move(memoryAllocation), image, info.type, info.format, info.extent,
      info.mip_levels, info.arrayLayers, info.samples)};
}

Blas MemoryPool::create_blas(const BlasInfo &info,
                             const MemoryLifetime &lifetime) {
  ZoneScopedN("MemoryPool::create_blas");
  auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);

  std::size_t geometryCount = 0;
  if (std::holds_alternative<span<const TriangleGeometryData>>(
          info.geometries)) {
    geometryCount =
        std::get<span<const TriangleGeometryData>>(info.geometries).size();
  } else {
    geometryCount =
        std::get<span<const AabbGeometryData>>(info.geometries).size();
  }
  assert(geometryCount > 0);
  assert(geometryCount <= std::numeric_limits<uint32_t>::max());

  Vector<Buffer> buffers;
  Vector<uint32_t> maxPrimitiveCounts{geometryCount};
  Vector<VkAccelerationStructureGeometryKHR> geometries{geometryCount};
  if (std::holds_alternative<span<const TriangleGeometryData>>(
          info.geometries)) {
    const auto sourceGeometries =
        std::get<span<const TriangleGeometryData>>(info.geometries);
    for (std::size_t i = 0; i < sourceGeometries.size(); ++i) {
      const TriangleGeometryData &geometry = sourceGeometries[i];

      geometry.positions.buffer.commit();
      const VkDeviceAddress positionAddress =
          void_handle_ptr<BufferImpl>(geometry.positions.buffer.m_handle)
              ->address +
          geometry.positions.offset;
      assert(positionAddress != 0);

      VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR;
      VkDeviceAddress indexAddress = 0;
      if (geometry.indices) {
        geometry.indices->buffer.commit();
        indexType = to_vk_index_type(geometry.indices->type);
        indexAddress =
            void_handle_ptr<BufferImpl>(geometry.indices->buffer.m_handle)
                ->address +
            geometry.indices->offset;
        assert(indexAddress != 0);
      }

      VkDeviceAddress transformAddress = 0;
      if (geometry.transform) {
        geometry.transform->buffer.commit();
        transformAddress =
            void_handle_ptr<BufferImpl>(geometry.transform->buffer.m_handle)
                ->address +
            geometry.transform->offset;
        assert(transformAddress != 0);
      }

      geometries[i] = VkAccelerationStructureGeometryKHR{
          .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
          .pNext = nullptr,
          .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
          .geometry =
              VkAccelerationStructureGeometryDataKHR{
                  .triangles =
                      VkAccelerationStructureGeometryTrianglesDataKHR{
                          .sType =
                              VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                          .pNext = nullptr,
                          .vertexFormat =
                              to_vk_format(geometry.positions.format),
                          .vertexData =
                              VkDeviceOrHostAddressConstKHR{
                                  .deviceAddress = positionAddress,
                              },
                          .vertexStride = geometry.positions.stride,
                          .maxVertex = geometry.positions.vertexCount == 0
                                           ? 0
                                           : geometry.positions.vertexCount - 1,
                          .indexType = indexType,
                          .indexData =
                              VkDeviceOrHostAddressConstKHR{
                                  .deviceAddress = indexAddress,
                              },
                          .transformData =
                              VkDeviceOrHostAddressConstKHR{
                                  .deviceAddress = transformAddress,
                              },
                      },
              },
          .flags = to_vk_geometry_flags(geometry.flags),
      };
      maxPrimitiveCounts[i] = geometry.maxTriangles;
      buffers.emplace_back(std::move(geometry.positions.buffer));
      if (geometry.indices.has_value()) {
        buffers.emplace_back(std::move(geometry.indices->buffer));
      }
      if (geometry.transform.has_value()) {
        buffers.emplace_back(std::move(geometry.transform->buffer));
      }
    }
  } else {
    const auto sourceGeometries =
        std::get<span<const AabbGeometryData>>(info.geometries);
    for (std::size_t i = 0; i < sourceGeometries.size(); ++i) {
      const AabbGeometryData &geometry = sourceGeometries[i];
      assert(geometry.buffer);
      geometry.buffer.commit();
      const VkDeviceAddress address =
          void_handle_ptr<BufferImpl>(geometry.buffer.m_handle)->address +
          geometry.offset;
      assert(address != 0);

      geometries[i] = VkAccelerationStructureGeometryKHR{
          .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
          .pNext = nullptr,
          .geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
          .geometry =
              VkAccelerationStructureGeometryDataKHR{
                  .aabbs =
                      VkAccelerationStructureGeometryAabbsDataKHR{
                          .sType =
                              VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
                          .pNext = nullptr,
                          .data =
                              VkDeviceOrHostAddressConstKHR{
                                  .deviceAddress = address,
                              },
                          .stride = geometry.stride,
                      },
              },
          .flags = to_vk_geometry_flags(geometry.flags),
      };
      maxPrimitiveCounts[i] = geometry.maxAabbs;
      buffers.emplace_back(std::move(geometry.buffer));
    }
  }
  assert(maxPrimitiveCounts.size() == geometries.size());

  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      .pNext = nullptr,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      .flags = to_vk_build_flags(info.flags),
      .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
      .srcAccelerationStructure = VK_NULL_HANDLE,
      .dstAccelerationStructure = VK_NULL_HANDLE,
      .geometryCount = static_cast<uint32_t>(geometries.size()),
      .pGeometries = geometries.data(),
      .ppGeometries = nullptr,
      .scratchData =
          VkDeviceOrHostAddressKHR{
              .deviceAddress = 0,
          },
  };

  VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
  sizeInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
  {
    ZoneScopedN("vkGetAccelerationStructureBuildSizes");
    vulkan::vk_get_acceleration_structure_build_sizes(
        impl->context.ctx()->pnf(), impl->context.ctx()->device(),
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
        maxPrimitiveCounts.data(), &sizeInfo);
  }

  Buffer buffer = create_buffer(
      {
          .size = sizeInfo.accelerationStructureSize,
          .bufferUsage = BufferUsage::acceleration_structure_storage |
                         BufferUsage::shader_device_address,
          .memoryUsage = info.memoryUsage,
      },
      lifetime);

  vulkan::AccelerationStructureInfo createInfo{
      .buffer = void_handle_ptr<BufferImpl>(buffer.m_handle)->buffer,
      .offset = 0,
      .size = sizeInfo.accelerationStructureSize,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
  };
  vulkan::AccelerationStructure accelerationStructure;
  {
    ZoneScopedN("vkCreateAccelerationStructure");
    accelerationStructure =
        vulkan::create_acceleration_structure(impl->context.ctx(), createInfo);
  }

  // reserve required build scratch space.
  VkDeviceSize requiredScratch =
      std::max(sizeInfo.buildScratchSize, sizeInfo.updateScratchSize);
  if (requiredScratch <= MemoryPoolImpl::MAX_SCRATCH_SIZE &&
      requiredScratch > impl->accelerationStructureScratch.size()) {

    auto [memory, buffer, internals] = impl->create_buffer(
        {
            .size = requiredScratch,
            .bufferUsage =
                BufferUsage::shader_device_address | BufferUsage::storage,
            .memoryUsage = MemoryUsage::automatic,
        },
        {});

    MemoryAllocation allocation{alloc_void_handle<
        MemoryAllocationImpl, memory_allocation_handle_allocator_ref>(
        impl->get_handle_alloc(), *this, memory, info.memoryUsage, internals)};
    Buffer scratchBuffer{alloc_void_handle<BufferImpl, buffer_handle_alloc_ref>(
        impl->get_buffer_handle_alloc(), impl->context, std::move(allocation),
        buffer, requiredScratch, 0)};
    scratchBuffer.commit();
    // NOTE: does this need to be synchronized!! (probably right)
    impl->accelerationStructureScratch = scratchBuffer;
  }

  return Blas{make_void_handle<BlasImpl>(
      impl->context, std::move(buffer), accelerationStructure, buildInfo.flags,
      std::move(maxPrimitiveCounts), std::move(geometries),
      sizeInfo.updateScratchSize, sizeInfo.buildScratchSize,
      std::move(buffers))};
}

void MemoryPool::commit() {
  ZoneScopedN("MemoryPool::commit");
  auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);
  impl->commit_all();
}

bool MemoryPool::memory_overlaps(const Buffer &lhs, const Buffer &rhs) const {
  ZoneScopedN("MemoryPool::memory_overlaps");
  // auto *impl = void_handle_ptr<MemoryPoolImpl>(m_handle);
  lhs.commit();
  rhs.commit();

  auto *lhs_impl =
      void_handle_ptr<BufferImpl, buffer_handle_alloc_ref>(lhs.m_handle);
  auto *lhs_mem_impl = void_handle_ptr<MemoryAllocationImpl,
                                       memory_allocation_handle_allocator_ref>(
      lhs_impl->allocation.m_handle);

  auto *rhs_impl =
      void_handle_ptr<BufferImpl, buffer_handle_alloc_ref>(lhs.m_handle);
  auto *rhs_mem_impl = void_handle_ptr<MemoryAllocationImpl,
                                       memory_allocation_handle_allocator_ref>(
      rhs_impl->allocation.m_handle);

  assert(lhs_mem_impl->binding);
  assert(rhs_mem_impl->binding);
  if (lhs_mem_impl->binding.memory.handle !=
      rhs_mem_impl->binding.memory.handle) {
    return false;
  }
  const VkDeviceSize lhs_offset = lhs_mem_impl->binding.offset;
  const VkDeviceSize lhs_size = lhs_impl->size;
  const VkDeviceSize rhs_offset = rhs_mem_impl->binding.offset;
  const VkDeviceSize rhs_size = rhs_impl->size;
  const bool overlap =
      lhs_offset < rhs_offset + rhs_size && rhs_offset < lhs_offset + lhs_size;
  return overlap;
}

} // namespace strobe::rhi
