#include "strobe/rhi/bvh/bvh.hpp"
#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/memory/memory_pool_impl.hpp"
#include "strobe/rhi/types/aabb.hpp"
#include "strobe/rhi/types/aabb_geometry_info_size_info.hpp"
#include "strobe/rhi/types/build_flags.hpp"
#include "strobe/rhi/types/triangle_geometry_size_info.hpp"
#include "strobe/rhi/utils/build_flags_utils.hpp"
#include "strobe/rhi/utils/format_utilts.hpp"
#include "strobe/rhi/utils/geometry_flags_utils.hpp"
#include "strobe/rhi/utils/index_type_utils.hpp"
#include "strobe/rhi/vulkan/acceleration_structure.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include <utility>
#include <variant>

namespace strobe::rhi::bvh {

static std::pair<BvhGeometryInfo, VkDeviceSize>
get_aabb_bvh_size(vulkan::Context *ctx, BuildFlags buildFlags,
                  span<const AabbGeometrySizeInfo> aabbs) {
  Vector<uint32_t> maxPrimitiveCounts{aabbs.size()};
  Vector<VkAccelerationStructureGeometryKHR> geometries{aabbs.size()};
  for (uint32_t i = 0; i < aabbs.size(); ++i) {
    geometries[i] = VkAccelerationStructureGeometryKHR{
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .pNext = nullptr,
        .geometryType = VK_GEOMETRY_TYPE_AABBS_KHR,
        .geometry =
            {
                .aabbs =
                    VkAccelerationStructureGeometryAabbsDataKHR{
                        .sType =
                            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR,
                        .pNext = nullptr,
                        .data = {.deviceAddress = 0},
                        .stride = sizeof(Aabb),
                    },
            },
        .flags = to_vk_geometry_flags(aabbs[i].flags),
    };
    maxPrimitiveCounts[i] = aabbs[i].maxAabbs;
  }
  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      .pNext = nullptr,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      .flags = to_vk_build_flags(buildFlags),
      .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
      .srcAccelerationStructure = VK_NULL_HANDLE,
      .dstAccelerationStructure = VK_NULL_HANDLE,
      .geometryCount = static_cast<uint32_t>(geometries.size()),
      .pGeometries = geometries.data(),
      .ppGeometries = nullptr,
      .scratchData = {.deviceAddress = 0},
  };
  VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
  sizeInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
  {
    ZoneScopedN("vkGetAccelerationStructureBuildSizes");
    vulkan::vk_get_acceleration_structure_build_sizes(
        ctx->pnf(), ctx->device(),
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
        maxPrimitiveCounts.data(), &sizeInfo);
  }

  Vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange{
      geometries.size()};
  std::memset(buildRange.data(), 0,
              buildRange.size() *
                  sizeof(VkAccelerationStructureBuildRangeInfoKHR));

  return {
      BvhGeometryInfo{
          .buildFlags = BuildFlags::none,
          .scratchSize =
              std::max(sizeInfo.buildScratchSize, sizeInfo.updateScratchSize),
          .maxPrimitiveCount = std::move(maxPrimitiveCounts),
          .geometries = std::move(geometries),
          .buildInfo = buildInfo,
          .buildRange = std::move(buildRange),
      },
      sizeInfo.accelerationStructureSize,
  };
}

static std::pair<BvhGeometryInfo, VkDeviceSize>
get_triangle_bvh_size(vulkan::Context *ctx, BuildFlags buildFlags,
                      span<const TriangleGeometrySizeInfo> triangles) {
  Vector<uint32_t> maxPrimitiveCounts{triangles.size()};
  Vector<VkAccelerationStructureGeometryKHR> geometries{triangles.size()};
  for (uint32_t i = 0; i < triangles.size(); ++i) {
    const auto &triangleGeometry = triangles[i];

    uint32_t maxVertex = triangleGeometry.maxVertexIndex;
    if (maxVertex == INFER_FROM_TRIANGLE_COUNT) {
      assert(triangleGeometry.indexType == IndexType::none);
      maxVertex = triangleGeometry.maxTriangles * 3 - 1;
    }

    geometries[i] = VkAccelerationStructureGeometryKHR{
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .pNext = nullptr,
        .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        .geometry =
            {
                .triangles =
                    VkAccelerationStructureGeometryTrianglesDataKHR{
                        .sType =
                            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                        .pNext = nullptr,
                        .vertexFormat =
                            to_vk_format(triangleGeometry.positionFormat),
                        .vertexData = {.deviceAddress = 0},
                        .vertexStride = sizeof(float[4]),
                        .maxVertex = maxVertex,
                        .indexType =
                            to_vk_index_type(triangleGeometry.indexType),
                        .indexData = {.deviceAddress = 0},
                        .transformData = {.deviceAddress =
                                              triangleGeometry.transform},
                    },
            },
        .flags = to_vk_geometry_flags(triangles[i].flags),
    };
    maxPrimitiveCounts[i] = triangles[i].maxTriangles;
  }

  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      .pNext = nullptr,
      .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      .flags = to_vk_build_flags(buildFlags),
      .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
      .srcAccelerationStructure = VK_NULL_HANDLE,
      .dstAccelerationStructure = VK_NULL_HANDLE,
      .geometryCount = static_cast<uint32_t>(geometries.size()),
      .pGeometries = geometries.data(),
      .ppGeometries = nullptr,
      .scratchData = {.deviceAddress = 0},
  };
  VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
  sizeInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
  {
    ZoneScopedN("vkGetAccelerationStructureBuildSizes");
    vulkan::vk_get_acceleration_structure_build_sizes(
        ctx->pnf(), ctx->device(),
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
        maxPrimitiveCounts.data(), &sizeInfo);
  }

  Vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange{
      geometries.size()};
  std::memset(buildRange.data(), 0,
              buildRange.size() *
                  sizeof(VkAccelerationStructureBuildRangeInfoKHR));

  return {
      BvhGeometryInfo{
          .buildFlags = BuildFlags::none,
          .scratchSize =
              std::max(sizeInfo.buildScratchSize, sizeInfo.updateScratchSize),
          .maxPrimitiveCount = std::move(maxPrimitiveCounts),
          .geometries = std::move(geometries),
          .buildInfo = buildInfo,
          .buildRange = std::move(buildRange),
      },
      sizeInfo.accelerationStructureSize,
  };
}

Blas create_blas(MemoryPool memoryPool, ScratchBuffer scratchBuffer,
                 const BlasInfo &info, const MemoryLifetime &lifetime,
                 handle_allocators *alloc) {
  Context context = memoryPool.context();
  vulkan::Context *ctx = context.ctx();

  std::pair<BvhGeometryInfo, VkDeviceSize> sizeInfo;
  const bool hasAabbGeometry =
      std::holds_alternative<span<const AabbGeometrySizeInfo>>(info.geometries);
  const bool hasTriangleGeometry =
      std::holds_alternative<span<const TriangleGeometrySizeInfo>>(
          info.geometries);

  if (hasAabbGeometry) {
    sizeInfo = get_aabb_bvh_size(
        ctx, info.buildFlags,
        std::get<span<const AabbGeometrySizeInfo>>(info.geometries));
  } else if (hasTriangleGeometry) {
    sizeInfo = get_triangle_bvh_size(
        ctx, info.buildFlags,
        std::get<span<const TriangleGeometrySizeInfo>>(info.geometries));
  } else {
    std::unreachable();
  }

  sizeInfo.first.buildFlags = info.buildFlags;

  Buffer buffer =
      buf::create_buffer(memoryPool,
                         {
                             .size = sizeInfo.second,
                             .bufferUsage = BufferUsage::storage |
                                            BufferUsage::shader_device_address,
                         },
                         lifetime, alloc->bufAllocators);
  auto *buf_impl = object_handle_ptr<BufferImpl>(buffer);

  vulkan::AccelerationStructure bvh = vulkan::create_acceleration_structure(
      ctx, {
               .buffer = buf_impl->buffer,
               .offset = 0,
               .size = buf_impl->size,
               .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
           });
  scratchBuffer.require(sizeInfo.first.scratchSize);

  return Blas{make_void_handle<BlasImpl>(
      &alloc->blasAllocator, std::move(context), std::move(buffer),
      std::move(scratchBuffer), bvh, std::move(sizeInfo.first))};
}

ScratchBuffer create_scratch(MemoryPool memoryPool, handle_allocators *alloc) {
  return ScratchBuffer{make_void_handle<ScratchBufferImpl>(
      &alloc->scratchBufferAllocator, std::move(memoryPool),
      alloc->bufAllocators)};
}

} // namespace strobe::rhi::bvh
