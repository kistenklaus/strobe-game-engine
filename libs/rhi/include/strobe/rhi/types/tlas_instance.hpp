#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief TLAS instance record.
 *
 * ABI-compatible representation of a top-level acceleration structure
 * instance and may be copied directly into TLAS build input buffers.
 */
struct TlasInstance {
  /** Object-to-world transform stored row-major as a 3x4 matrix. */
  float transform[3][4];

  /** User-defined instance index. */
  uint32_t customIndex : 24;

  /** Instance visibility mask. */
  uint32_t mask : 8;

  /** Shader binding table record offset. */
  uint32_t shaderBindingTableRecordOffset : 24;

  /** Instance behavior flags. */
  uint32_t flags : 8;

  /** Device address of the referenced BLAS. */
  uint64_t blasAddress;
};

} // namespace strobe::rhi
