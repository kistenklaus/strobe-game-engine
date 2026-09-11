#pragma once

#include <cstdint>

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Device memory usage intend
 */
enum class MemoryUsage : uint8_t {
  /**
   * \brief Automatically select an appropriate memory usage.
   *
   * The allocator chooses the memory type based on the resource requirements
   */
  automatic,
  /**
   * \brief Device-local memory.
   *
   * Intended for resources which have to be on the device, otherwise
   * prefer automatic.
   */
  device,
  /**
   * \brief Host-mapped memory.
   *
   * Intended for resources that require general-purpose host access.
   * Primarily useful for download staging buffers.
   */
  mapped,
  /**
   * \brief Host-mapped memory optimized for sequential writes.
   *
   * Intended for resources that are predominantly written sequentially by
   * the host and consumed by the device.
   * Primarily useful for upload staging buffers.
   */
  mapped_write_sequential,
  /**
   * \brief Host-mapped non-coherent memory.
   *
   * Host writes may require explicit flushing before they become visible to
   * the device.
   */
  mapped_incoherent,
};

} // namespace strobe::rhi
