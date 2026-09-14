#pragma once

namespace strobe::rhi {

/**
 * \ingroup rhi
 * \brief Fence creation information.
 * Defined in header <strobe/rhi/rhi.hpp>
 * \snippet{cpp} fence_info.hpp FenceInfo
 *
 * Describes the initial state of a fence.
 */
// [FenceInfo]
struct FenceInfo {
  bool signaled = false;
};
// [FenceInfo]

} // namespace strobe::rhi
