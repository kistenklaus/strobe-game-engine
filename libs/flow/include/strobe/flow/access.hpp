#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/resource.hpp"
#include "strobe/flow/type_id.hpp"

#include <type_traits>
#include <utility>

namespace strobe::flow {

namespace detail {
template <typename T, AccessMode Mode> class Access {
  friend class OpStream;

public:
  explicit Access(Resource<T> resource) : m_resource(std::move(resource)) {}

private:
  static constexpr TypeId argument_type = type_id<T>();

  static auto &get(void *storage) {
    if constexpr (Mode == AccessMode::read) {
      return std::as_const(*static_cast<T *>(storage));
    } else {
      return *static_cast<T *>(storage);
    }
  }
  Resource<T> m_resource;
};

template <typename T> struct IsAccess : std::false_type {};

template <typename T, AccessMode Mode>
struct IsAccess<Access<T, Mode>> : std::true_type {};

template <typename T>
inline constexpr bool is_access = IsAccess<std::remove_cvref_t<T>>::value;
} // namespace detail

/**
 * \brief Declares a read-only resource access for an operation.
 * \code{.cpp}
 * template<typename T>
 * auto read(Resource<T> resource);
 * \endcode
 *
 * \param resource Resource read by the operation.
 * \return A read access descriptor.
 */
template <typename T> auto read(Resource<T> resource) {
  return detail::Access<T, AccessMode::read>(std::move(resource));
}

/**
 * \brief Declares a resource write that may preserve part of its current value.
 * \code{.cpp}
 * template<typename T>
 * auto write(Resource<T> resource);
 * \endcode
 *
 * \param resource Resource written by the operation.
 * \return A write access descriptor.
 */
template <typename T> auto write(Resource<T> resource) {
  return detail::Access<T, AccessMode::write>(std::move(resource));
}

/**
 * \brief Declares an explicit read and write access to a resource.
 * \code{.cpp}
 * template<typename T>
 * auto read_write(Resource<T> resource);
 * \endcode
 *
 * \param resource Resource read and written by the operation.
 * \return A read-write access descriptor.
 */
template <typename T> auto read_write(Resource<T> resource) {
  return detail::Access<T, AccessMode::read_write>(std::move(resource));
}

} // namespace strobe::flow
