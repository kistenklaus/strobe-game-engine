#pragma once

#include <cstdint>

namespace strobe::flow {

/**
 * \brief Identifies a half-open interval in a compiled Flow schedule.
 * \code{.cpp}
 * struct Lifetime {
 *   std::uint32_t begin;
 *   std::uint32_t end;
 * };
 * \endcode
 *
 * The positions are logical schedule indices. They are independent of the
 * resource's byte size, alignment, and storage offset.
 */
struct Lifetime {
  std::uint32_t begin;
  std::uint32_t end;
};

static_assert(sizeof(Lifetime) == sizeof(std::uint32_t) * 2);

} // namespace strobe::flow
