#pragma once

#include <cstddef>

namespace strobe {

/**
 * \ingroup core
 * \brief Returns the operating system memory page size.
 * \code{.cpp}
 * std::size_t page_size();
 * \endcode
 *
 * \return The system page size in bytes.
 */
std::size_t page_size();

}
