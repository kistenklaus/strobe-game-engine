#pragma once

#include "strobe/flow/compiler_plugins.hpp"

namespace strobe::rdg {

/**
 * \brief Appends the RDG compiler passes to a Flow plugin table.
 * \code{.cpp}
 * void append_plugins(flow::CompilerPlugins& plugins);
 * \endcode
 *
 * \param plugins Plugin table that receives the RDG compiler passes.
 */
void append_plugins(flow::CompilerPlugins &plugins);

} // namespace strobe::rdg
