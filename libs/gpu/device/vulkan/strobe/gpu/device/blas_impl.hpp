#pragma once

#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/context.hpp"
#include "strobe/gpu/vulkan/acceleration_structure.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::gpu {

struct BlasImpl {

  BlasImpl(Context context, Buffer buffer,
           vulkan::AccelerationStructure accelerationStructure,
           VkBuildAccelerationStructureFlagsKHR buildFlags,
           Vector<uint32_t> maxPrimitiveCounts,
           Vector<VkAccelerationStructureGeometryKHR> geometries,
           VkDeviceSize updateScratchSize, 
           VkDeviceSize buildScratchSize,
           Vector<Buffer> boundBuffers) noexcept
      : context(std::move(context)), buffer(std::move(buffer)),
        accelerationStructure(accelerationStructure), buildFlags(buildFlags),
        maxPrimitiveCount(std::move(maxPrimitiveCounts)),
        geometries(std::move(geometries)),
        updateScratchSize(updateScratchSize),
        buildScratchSize(buildScratchSize),
        boundBuffers(std::move(boundBuffers)) {
    assert(this->buffer);
    assert(this->accelerationStructure);
  }

  ~BlasImpl() noexcept {
    assert(this->buffer);
    assert(this->accelerationStructure);
    vulkan::destroy_acceleration_structure(context.get(),
                                           accelerationStructure);
  }

  const Context context;
  const Buffer buffer;
  const vulkan::AccelerationStructure accelerationStructure;

  // build geometry info
  VkBuildAccelerationStructureFlagsKHR buildFlags;
  const Vector<uint32_t> maxPrimitiveCount;
  const Vector<VkAccelerationStructureGeometryKHR> geometries;
  // build sizes
  VkDeviceSize updateScratchSize;
  VkDeviceSize buildScratchSize;
  // bound buffers
  Vector<Buffer> boundBuffers;
};

} // namespace strobe::gpu
