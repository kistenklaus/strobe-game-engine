#pragma once

namespace strobe::flow {

/**
 * \brief Identifies whether an access supplies an argument or only ordering.
 * \code{.cpp}
 * enum class AccessPurpose;
 * \endcode
 */
enum class AccessPurpose {
  argument,
  serialization,
};

} // namespace strobe::flow
