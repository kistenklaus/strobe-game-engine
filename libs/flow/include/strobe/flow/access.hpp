#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/resource.hpp"
#include "strobe/flow/type_id.hpp"

#include <strobe/core/containers/span.hpp>

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

template <typename T> class SerializationAccess {
  friend class OpStream;

public:
  explicit SerializationAccess(Resource<T> resource)
      : m_resource(std::move(resource)) {}

private:
  Resource<T> m_resource;
};

template <typename T, AccessMode Mode> class AccessRange {
  friend class OpStream;

public:
  explicit AccessRange(span<const Resource<T>> resources)
      : m_resources(resources) {}

private:
  span<const Resource<T>> m_resources;
};

template <typename T> struct IsAccess : std::false_type {};

template <typename T, AccessMode Mode>
struct IsAccess<Access<T, Mode>> : std::true_type {};

template <typename T>
inline constexpr bool is_access = IsAccess<std::remove_cvref_t<T>>::value;

template <typename T> struct IsAccessRange : std::false_type {};

template <typename T, AccessMode Mode>
struct IsAccessRange<AccessRange<T, Mode>> : std::true_type {};

template <typename T>
inline constexpr bool is_access_range =
    IsAccessRange<std::remove_cvref_t<T>>::value;

template <typename T> struct IsSerializationAccess : std::false_type {};

template <typename T>
struct IsSerializationAccess<SerializationAccess<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_serialization_access =
    IsSerializationAccess<std::remove_cvref_t<T>>::value;

template <typename T>
inline constexpr bool is_any_access =
    is_access<T> || is_serialization_access<T>;

template <typename T>
inline constexpr bool is_recorded_access =
    is_access<T> || is_access_range<T> || is_serialization_access<T>;
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

/**
 * \brief Declares an exclusive final access to a resource value.
 * \code{.cpp}
 * template<typename T>
 * auto consume(Resource<T> resource);
 * \endcode
 *
 * The callback receives the current object as `T&`. The access produces no
 * replacement value and leaves the resource undefined after the operation.
 *
 * \param resource Resource whose current value is consumed.
 * \return A consuming access descriptor.
 * \attention 1 The resource cannot be accessed again until a new value is
 * established.
 */
template <typename T> auto consume(Resource<T> resource) {
  return detail::Access<T, AccessMode::consume>(std::move(resource));
}

/**
 * \brief Declares a fixed group of read-only resource accesses.
 * \code{.cpp}
 * template<typename T>
 * auto read_span(span<const Resource<T>> resources);
 * \endcode
 *
 * \param resources Resources grouped into one callback argument.
 * \return A grouped read access descriptor.
 */
template <typename T> auto read_span(span<const Resource<T>> resources) {
  return detail::AccessRange<T, AccessMode::read>(resources);
}

/**
 * \brief Declares a fixed group of resource writes.
 * \code{.cpp}
 * template<typename T>
 * auto write_span(span<const Resource<T>> resources);
 * \endcode
 *
 * \param resources Resources grouped into one callback argument.
 * \return A grouped write access descriptor.
 */
template <typename T> auto write_span(span<const Resource<T>> resources) {
  return detail::AccessRange<T, AccessMode::write>(resources);
}

/**
 * \brief Declares a fixed group of read-write resource accesses.
 * \code{.cpp}
 * template<typename T>
 * auto read_write_span(span<const Resource<T>> resources);
 * \endcode
 *
 * \param resources Resources grouped into one callback argument.
 * \return A grouped read-write access descriptor.
 */
template <typename T> auto read_write_span(span<const Resource<T>> resources) {
  return detail::AccessRange<T, AccessMode::read_write>(resources);
}

/**
 * \brief Declares a fixed group of consuming resource accesses.
 * \code{.cpp}
 * template<typename T>
 * auto consume_span(span<const Resource<T>> resources);
 * \endcode
 *
 * \param resources Resources grouped into one callback argument.
 * \return A grouped consuming access descriptor.
 */
template <typename T> auto consume_span(span<const Resource<T>> resources) {
  return detail::AccessRange<T, AccessMode::consume>(resources);
}

/**
 * \brief Declares an exclusive ordering dependency without a callback argument.
 * \code{.cpp}
 * template<typename T>
 * auto serialize(Resource<T> resource);
 * \endcode
 *
 * \param resource Resource used as the serialization domain.
 * \return A trailing serialization access descriptor.
 * \attention 1 Serialization accesses must follow every ordinary access.
 */
template <typename T> auto serialize(Resource<T> resource) {
  return detail::SerializationAccess<T>(std::move(resource));
}

} // namespace strobe::flow
