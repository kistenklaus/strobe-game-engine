#pragma once

#include "strobe/flow/resource.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"

namespace strobe::rdg {

class Buffer;

namespace intr {

inline void decl_buffer_state(Buffer buffer, rhi::PipelineStage stage,
                              rhi::Access access);

}

class Buffer {
public:
  friend class CommandBuffer;
  friend void intr::decl_buffer_state(Buffer buffer, rhi::PipelineStage stage,
                                      rhi::Access access);

private:
  explicit Buffer(flow::Resource<rhi::Buffer> runtime) noexcept
      : m_runtime(runtime) {}
  flow::Resource<rhi::Buffer> m_runtime;
};

} // namespace strobe::rdg
