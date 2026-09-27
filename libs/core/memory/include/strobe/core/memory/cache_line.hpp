#pragma once

namespace strobe::memory {

/**
 * \ingroup core
 * \brief Assumed cache-line size in bytes.
 * \code{.cpp}
 * static constexpr int cache_line = 64;
 * \endcode
 *
 * The constant is used when selecting cache-line-aware layouts and padding.
 */
static constexpr int cache_line = 64;

}
