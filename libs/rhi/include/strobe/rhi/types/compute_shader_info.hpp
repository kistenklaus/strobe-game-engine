#pragma once

#include "strobe/core/containers/span.hpp"
#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Compute shader creation information.
 *
 * Describes the SPIR-V module used to create a compute shader.
 */
struct ComputeShaderInfo {
  /** SPIR-V shader code. */
  span<const uint32_t> spirv = {};
};

} // namespace strobe::rhi
