#pragma once

#include <cstddef>
#include <limits>
namespace strobe {

/**
 * \ingroup core
 * \brief Sentinel value representing an invalid container index.
 * \code{.cpp}
 * static constexpr size_t INVALID_INDEX;
 * \endcode
 */
static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

}
