#pragma once

#include <cstdint>
#include <limits>

namespace strobe::flow {

/**
 * \brief Identifies one logical argument within a flat runtime argument list.
 * \code{.cpp}
 * struct ArgumentGroup {
 *   std::uint32_t offset;
 *   std::uint32_t count;
 *   std::uint32_t element_offset;
 *   std::uint32_t element_count;
 * };
 * \endcode
 */
struct ArgumentGroup {
  std::uint32_t offset = 0;
  std::uint32_t count = 0;
  std::uint32_t element_offset = 0;
  std::uint32_t element_count = 0;
};

/**
 * \brief Marks an absent element in a logical argument group.
 * \code{.cpp}
 * inline constexpr std::uint32_t null_argument_element;
 * \endcode
 */
inline constexpr std::uint32_t null_argument_element =
    std::numeric_limits<std::uint32_t>::max();

} // namespace strobe::flow
