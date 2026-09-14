#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Device memory usage intend
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} memory_usage.hpp MemoryUsage
 */
// [MemoryUsage]
enum class MemoryUsage : uint8_t {
  automatic,
  device,
  mapped,
  mapped_write_sequential,
  mapped_incoherent,
};
// [MemoryUsage]

} // namespace strobe::rhi
