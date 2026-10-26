#pragma once

#include <cstdint>

namespace strobe::flow {

/**
 * \brief Configures execution of an externally driven feedback loop.
 * \code{.cpp}
 * struct FeedbackOptions {
 *   std::uint32_t max_in_flight = 2;
 * };
 * \endcode
 */
struct FeedbackOptions {
  std::uint32_t max_in_flight = 2;
};

} // namespace strobe::flow
