#pragma once

#include "strobe/rhi/types/shader_stage.hpp"

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Push constant range.
 *
 * Describes a byte range of push constants accessible from selected shader
 * stages.
 */
struct PushConstantRange {
  /** Shader stages that may access the range. */
  ShaderStage stage;

  /** Byte offset of the range. */
  uint32_t offset;

  /** Size of the range in bytes. */
  uint32_t size;
};

} // namespace strobe::rhi
