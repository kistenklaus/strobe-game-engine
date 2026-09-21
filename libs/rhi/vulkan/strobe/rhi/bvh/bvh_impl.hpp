#pragma once

#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/bvh/bvh_geometry_info.hpp"
#include "strobe/rhi/bvh/scratch_buffer.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/vulkan/acceleration_structure.hpp"
#include <mutex>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

struct BvhImpl {

  explicit BvhImpl(Context context, Buffer buffer, ScratchBuffer scratchBuffer,
                   vulkan::AccelerationStructure accelerationStructure,
                   BvhGeometryInfo geometryInfo) noexcept
      : context(std::move(context)), buffer(std::move(buffer)),
        scratchBuffer(std::move(scratchBuffer)),
        accelerationStructure(accelerationStructure),
        m_geometryInfo(std::move(geometryInfo)) {
    assert(this->buffer);
    assert(this->accelerationStructure);
  }

  ~BvhImpl() noexcept {
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

  uint32_t maxPrimitiveCount(uint32_t geo) const noexcept {
    assert(geo < m_geometryInfo.maxPrimitiveCount.size());
    return m_geometryInfo.maxPrimitiveCount[geo];
  }

  VkDeviceAddress address() {
    std::lock_guard lck{m_geometryInfoMutex};
    if (m_address == 0) {
      object_handle_ptr<BufferImpl>(buffer)->commit();
      m_address = vulkan::get_acceleration_structure_device_address(
          context.ctx(), accelerationStructure);
    }
    return m_address;
  }

  const Context context;
  const Buffer buffer;
  const ScratchBuffer scratchBuffer;
  const vulkan::AccelerationStructure accelerationStructure;

private:
  VkDeviceAddress m_address = 0;

  BvhGeometryInfo m_geometryInfo;
  std::mutex m_geometryInfoMutex{};
};

} // namespace strobe::rhi
