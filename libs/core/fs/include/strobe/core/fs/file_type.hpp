#pragma once

namespace strobe::fs {

/**
 * \brief Type of a filesystem entry.
 * \ingroup core
 * \snippet{.cpp} file_type.hpp Type
 */
// [Type]
enum class Type { File, Directory, SymLink };
// [Type]

} // namespace strobe::fs
