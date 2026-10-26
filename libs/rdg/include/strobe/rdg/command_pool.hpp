#pragma once

#include "strobe/flow/access.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/resource.hpp"
#include "strobe/rdg/command_buffer.hpp"
namespace strobe::rdg {

class CommandPool {
public:
  CommandPool() : m_runtime(flow::resource<rhi::CommandPool>()) {
    flow::construct(m_runtime, flow::intrinsic("strobe.rdg", "create_cmdpool"));
  }

  CommandBuffer alloc() {
    flow::Resource<rhi::CommandBuffer> cmd =
        flow::resource<rhi::CommandBuffer>();
    flow::construct(
        cmd, flow::OpMetadata{"alloc_cmd", "allocate a command buffer"}, flow::read_write(m_runtime),
        [](rhi::CommandBuffer *cmd, rhi::CommandPool &pool) noexcept {
          std::construct_at(cmd, pool.alloc(rhi::CommandBufferFlags::none));
          cmd->begin();
        });
    return CommandBuffer{cmd, m_runtime};
  }

private:
  flow::Resource<rhi::CommandPool> m_runtime;
};

} // namespace strobe::rdg
