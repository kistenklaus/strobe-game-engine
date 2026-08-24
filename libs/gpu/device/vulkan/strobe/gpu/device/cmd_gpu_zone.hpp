#pragma once

#ifdef TRACY_ENABLE
#include "strobe/gpu/device/context.hpp"
#include <client/TracyProfiler.hpp>
#include <memory>
#endif

namespace strobe::gpu {

#ifdef TRACY_ENABLE
class CmdGpuZone {
public:
  CmdGpuZone(TracyVkCtx context,
             const tracy::SourceLocationData *sourceLocation,
             VkCommandBuffer commandBuffer, bool active) noexcept {
    ZoneScopedN("Tracy GPU zone begin");
    std::construct_at(&m_storage.zone, context, sourceLocation, commandBuffer,
                      active);
  }

  ~CmdGpuZone() noexcept {
    ZoneScopedN("Tracy GPU zone end");
    std::destroy_at(&m_storage.zone);
  }

  CmdGpuZone(const CmdGpuZone &) = delete;
  CmdGpuZone &operator=(const CmdGpuZone &) = delete;
  CmdGpuZone(CmdGpuZone &&) = delete;
  CmdGpuZone &operator=(CmdGpuZone &&) = delete;

private:
  union Storage {
    constexpr Storage() noexcept {}
    ~Storage() {}

    tracy::VkCtxScope zone;
  } m_storage;
};
#endif

} // namespace strobe::gpu
