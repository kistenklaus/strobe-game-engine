#pragma once

#include <cstdint>
#include <limits>

namespace strobe::flow {

/**
 * \brief Untyped identifier for a resource within one recording or program.
 * \code{.cpp}
 * using ResourceId = uint32_t;
 * \endcode
 */
using ResourceId = uint32_t;

/**
 * \brief Sentinel that does not identify a resource.
 * \code{.cpp}
 * inline constexpr ResourceId null_resource_id;
 * \endcode
 */
inline constexpr ResourceId null_resource_id =
    std::numeric_limits<ResourceId>::max();

} // namespace strobe::flow
