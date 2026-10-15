#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/resource_id.hpp"

namespace strobe::flow {

namespace detail {
class OpStream;
template <typename T, AccessMode Mode> class Access;
} // namespace detail

/**
 * \brief Identifies a typed resource declared in the active recording.
 * \code{.cpp}
 * template<typename T>
 * class Resource;
 * \endcode
 *
 * Resource handles are created by Flow recording functions and are valid only
 * for the recording that created them.
 */
template <typename T> class Resource {
  friend class detail::OpStream;
  template <typename, AccessMode> friend class detail::Access;

  explicit Resource(ResourceId id) : m_id(id) {}

  ResourceId m_id = null_resource_id;
};

} // namespace strobe::flow
