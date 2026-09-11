#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Fence creation information.
 *
 * Describes the initial state of a fence.
 */
struct FenceInfo {
  /** Whether the fence is initially signaled. */
  bool signaled = false;
};

} // namespace strobe::rhi
