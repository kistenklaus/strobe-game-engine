#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief TLAS instance record.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} tlas_instance.hpp TlasInstance
 *
 * ABI-compatible representation of a top-level acceleration structure
 * instance and may be copied directly into TLAS build input buffers.
 */
// [TlasInstance]
struct TlasInstance {
  float transform[3][4];
  uint32_t customIndex : 24;
  uint32_t mask : 8;
  uint32_t shaderBindingTableRecordOffset : 24;
  uint32_t flags : 8;
  uint64_t blasAddress;
};
// [TlasInstance]

} // namespace strobe::rhi
