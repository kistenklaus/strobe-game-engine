#include "strobe/rhi/vulkan/cmd/dispatch.hpp"

void strobe::rhi::vulkan::cmd_dispatch(CommandBuffer cmd, uint32_t groupCountX,
                                       uint32_t groupCountY,
                                       uint32_t groupCountZ) noexcept {
  {
#ifdef STROBE_RHI_TRACE_VK
    ZoneScopedN("vkCmdDispatch");
#endif
    vkCmdDispatch(cmd.handle, groupCountX, groupCountY, groupCountZ);
  }
}
