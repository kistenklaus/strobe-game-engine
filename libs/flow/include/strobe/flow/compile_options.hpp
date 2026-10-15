#pragma once

#include <cstdint>

namespace strobe::flow {

/**
 * \brief Configures bounded runtime storage and activation overlap.
 * \code{.cpp}
 * struct CompileOptions {
 *   std::uint32_t max_slots = 2;
 * };
 * \endcode
 */
struct CompileOptions {
  std::uint32_t max_slots = 2;
};

} // namespace strobe::flow
