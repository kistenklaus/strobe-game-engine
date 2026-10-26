#pragma once

#include "strobe/flow/access.hpp"
#include "strobe/flow/flow.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/rdg/buffer.hpp"
#include "strobe/rdg/image.hpp"

namespace strobe::rdg::intr {

inline void decl_buffer_state(flow::Resource<rhi::Buffer> buffer,
                              rhi::PipelineStage stage, rhi::Access access) {
  flow::op(flow::intrinsic("strobe.rdg", "decl-buffer-state", 1)
               .immediate(stage)   //
               .immediate(access), //
           flow::read(buffer));    //
}

inline void decl_buffer_state(Buffer buffer, rhi::PipelineStage stage,
                              rhi::Access access) {
  decl_buffer_state(buffer.m_runtime, stage, access);
}

inline void decl_image_state(flow::Resource<rhi::Image> image,
                             rhi::PipelineStage stage, rhi::Access access,
                             rhi::ImageLayout layout) {
  flow::op(flow::intrinsic("strobe.rdg", "decl-image-state", 1)
               .immediate(stage)   //
               .immediate(access)  //
               .immediate(layout), //
           flow::read(image));     //
}

inline void decl_image_state(Image image, rhi::PipelineStage stage,
                             rhi::Access access, rhi::ImageLayout layout) {
  decl_image_state(image.m_runtime, stage, access, layout);
}

} // namespace strobe::rdg::intr
