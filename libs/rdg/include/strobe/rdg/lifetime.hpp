#pragma once

#include <strobe/flow/lifetime.hpp>
#include <strobe/rhi/types/memory_lifetime.hpp>

#include <limits>

namespace strobe::rdg {

/**
 * \brief Converts a Flow schedule lifetime to an RHI memory lifetime.
 * \code{.cpp}
 * constexpr rhi::MemoryLifetime
 * memory_lifetime(flow::Lifetime lifetime) noexcept;
 * \endcode
 *
 * \param lifetime Half-open logical lifetime supplied by Flow.
 * \return The equivalent inclusive RHI memory lifetime.
 * \attention 1 The input lifetime must not be empty.
 */
constexpr rhi::MemoryLifetime
memory_lifetime(flow::Lifetime lifetime) noexcept {
  return rhi::MemoryLifetime{
      .begin = lifetime.begin,
      .end = lifetime.end == std::numeric_limits<std::uint32_t>::max()
                 ? lifetime.end
                 : lifetime.end - 1,
  };
}

} // namespace strobe::rdg
