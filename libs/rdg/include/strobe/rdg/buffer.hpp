#pragma once

#include "strobe/flow/access.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/resource.hpp"
#include "strobe/rdg/types/buffer_info.hpp"
#include "strobe/rdg/types/buffer_info_immediates.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/types/access.hpp"
#include "strobe/rhi/types/buffer_info.hpp"
#include "strobe/rhi/types/pipeline_stage.hpp"
#include <memory>

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

  Buffer(const BufferInfo &info) noexcept
      : m_runtime(flow::resource<rhi::Buffer>()) {

    BufferInfoImmediates imm{
        .bufferUsage = info.bufferUsage,
        .memoryUsage = info.memoryUsage,
    };

    flow::construct(
        m_runtime,
        flow::intrinsic("strobe.rdg", "create_buffer", 1).immediate(imm),
        flow::read(info.size.runtime()));
  }

  flow::u64 size() const noexcept {
    auto sz = flow::resource<uint64_t>();
    flow::construct(sz, flow::read(m_runtime),
                    [](uint64_t *ptr, const rhi::Buffer &buf) noexcept {
                      std::construct_at(ptr, buf.size());
                    });
    return sz;
  }

private:
  // explicit Buffer(flow::Resource<rhi::Buffer> runtime) noexcept
  //     : m_runtime(runtime) {}
  flow::Resource<rhi::Buffer> m_runtime;
};

} // namespace strobe::rdg
