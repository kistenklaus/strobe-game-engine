#pragma once

namespace strobe::flow {

/**
 * \brief Identifies how an operation accesses a resource.
 * \code{.cpp}
 * enum class AccessMode;
 * \endcode
 */
enum class AccessMode {
  read,
  write,
  read_write,
};

} // namespace strobe::flow
