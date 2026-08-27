#pragma once

#include "strobe/rhi/bvh/bvh_geometry_info.hpp"
#include "strobe/rhi/bvh/scratch_buffer.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/vulkan/acceleration_structure.hpp"
#include <mutex>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct BlasImpl {

  explicit BlasImpl(Context context, Buffer buffer, ScratchBuffer scratchBuffer,
                    vulkan::AccelerationStructure accelerationStructure,
                    BvhGeometryInfo geometryInfo) noexcept
      : context(std::move(context)), buffer(std::move(buffer)),
        scratchBuffer(std::move(scratchBuffer)),
        accelerationStructure(accelerationStructure),
        m_geometryInfo(std::move(geometryInfo)) {
    assert(this->buffer);
    assert(this->accelerationStructure);
  }

  ~BlasImpl() noexcept {
    assert(this->buffer);
    assert(this->accelerationStructure);
    vulkan::destroy_acceleration_structure(context.ctx(),
                                           accelerationStructure);
  }

  std::lock_guard<std::mutex> lockBuildInfo() { // URVO
    return std::lock_guard{m_geometryInfoMutex};
  }

  std::pair<VkAccelerationStructureBuildGeometryInfoKHR *,
            VkAccelerationStructureBuildRangeInfoKHR *>
  buildInfo() {
    return std::make_pair(&m_geometryInfo.buildInfo,
                          m_geometryInfo.buildRange.data());
  }

  const Context context;
  const Buffer buffer;
  const ScratchBuffer scratchBuffer;
  const vulkan::AccelerationStructure accelerationStructure;

private:
  BvhGeometryInfo m_geometryInfo;
  std::mutex m_geometryInfoMutex{};
};

} // namespace strobe::rhi
