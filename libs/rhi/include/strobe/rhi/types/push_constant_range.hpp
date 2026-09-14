#pragma once

#include "strobe/rhi/types/shader_stage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Push constant range.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} push_constant_range.hpp PushConstantRange
 *
 * Describes a byte range of push constants accessible from selected shader
 * stages.
 */
// [PushConstantRange]
struct PushConstantRange {
  ShaderStage stage;
  uint32_t offset;
  uint32_t size;
};
// [PushConstantRange]

} // namespace strobe::rhi
