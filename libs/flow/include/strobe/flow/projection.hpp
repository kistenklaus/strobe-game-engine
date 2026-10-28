#pragma once

#include "strobe/flow/op_source.hpp"

#include <cstdint>

namespace strobe::flow {

/**
 * \brief Selects how sibling projections of one owner synchronize.
 * \code{.cpp}
 * enum class ProjectionKind : std::uint8_t;
 * \endcode
 */
enum class ProjectionKind : std::uint8_t {
  disjoint,
  overlapping,
};

/**
 * \brief Identifies a projector resolved from an external Flow library.
 * \code{.cpp}
 * struct ExternalProjector;
 * \endcode
 */
struct ExternalProjector {
  ExternalOp source;
};

/**
 * \brief Creates an external projector source.
 * \code{.cpp}
 * ExternalProjector external_projector(std::string_view library_id,
 *                                      std::string_view symbol,
 *                                      std::uint32_t abi_version);
 * \endcode
 *
 * \param library_id Bound library that provides the projector.
 * \param symbol Projector symbol in the library.
 * \param abi_version Projector ABI version.
 * \return The deferred projector source.
 */
inline ExternalProjector external_projector(std::string_view library_id,
                                            std::string_view symbol,
                                            std::uint32_t abi_version) {
  return {.source = external_op(library_id, symbol, abi_version)};
}

} // namespace strobe::flow
