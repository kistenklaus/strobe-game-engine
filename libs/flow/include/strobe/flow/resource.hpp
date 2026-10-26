#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/resource_id.hpp"

namespace strobe::flow {

/**
 * \brief Identifies an absent typed resource argument.
 * \code{.cpp}
 * struct NullResource;
 * \endcode
 */
struct NullResource {};

/**
 * \brief Provides a type-independent null resource value.
 * \code{.cpp}
 * inline constexpr NullResource null_resource;
 * \endcode
 */
inline constexpr NullResource null_resource{};

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

public:
  /**
   * \brief Constructs a null resource handle.
   * \code{.cpp}
   * Resource() noexcept;
   * Resource(NullResource) noexcept;
   * \endcode
   */
  Resource() noexcept = default;
  Resource(NullResource) noexcept {}

  /**
   * \brief Tests whether the handle identifies a recorded resource.
   * \code{.cpp}
   * explicit operator bool() const noexcept;
   * \endcode
   *
   * \return Whether the handle is non-null.
   */
  explicit operator bool() const noexcept { return m_id != null_resource_id; }

private:
  explicit Resource(ResourceId id) : m_id(id) {}

  ResourceId m_id = null_resource_id;
};

} // namespace strobe::flow
