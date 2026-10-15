#pragma once

#include <strobe/flow/access.hpp>
#include <strobe/rhi/types/access_scope.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

namespace strobe::rdg {

namespace detail {
template <typename T> struct IsScopedAccess : std::false_type {};
} // namespace detail

/**
 * \brief Associates a Flow resource access with an RHI access scope.
 * \code{.cpp}
 * template<typename Access>
 * class ScopedAccess;
 * \endcode
 *
 * \tparam Access Flow access descriptor type.
 */
template <typename Access> class ScopedAccess {
public:
  /**
   * \brief Constructs a scoped device-resource access.
   * \code{.cpp}
   * ScopedAccess(Access access, rhi::AccessScope scope);
   * \endcode
   *
   * \param access Flow resource access.
   * \param scope RHI pipeline and memory access scope.
   */
  ScopedAccess(Access access, rhi::AccessScope scope)
      : m_access(std::move(access)), m_scope(scope) {}

  /**
   * \brief Releases the underlying Flow access.
   * \code{.cpp}
   * Access release() &&;
   * \endcode
   *
   * \return Underlying Flow access descriptor.
   */
  Access release() && { return std::move(m_access); }

  /**
   * \brief Returns the associated RHI access scope.
   * \code{.cpp}
   * rhi::AccessScope scope() const noexcept;
   * \endcode
   *
   * \return Recorded RHI access scope.
   */
  rhi::AccessScope scope() const noexcept { return m_scope; }

private:
  Access m_access;
  rhi::AccessScope m_scope;
};

namespace detail {
template <typename Access>
struct IsScopedAccess<ScopedAccess<Access>> : std::true_type {};

template <typename T>
inline constexpr bool is_scoped_access =
    IsScopedAccess<std::remove_cvref_t<T>>::value;

template <typename Tuple, std::size_t... I>
consteval bool scoped_access_prefix(std::index_sequence<I...>) {
  return (is_scoped_access<std::tuple_element_t<I, Tuple>> && ...);
}
} // namespace detail

/**
 * \brief Records a read access with an RHI access scope.
 * \code{.cpp}
 * template<typename T>
 * auto read(flow::Resource<T> resource, rhi::AccessScope scope);
 * \endcode
 *
 * \param resource Resource read by an RDG operation.
 * \param scope RHI scope in which the read occurs.
 * \return Scoped Flow read descriptor.
 */
template <typename T>
auto read(flow::Resource<T> resource, rhi::AccessScope scope) {
  auto access = flow::read(resource);
  return ScopedAccess<decltype(access)>(std::move(access), scope);
}

/**
 * \brief Records a write access with an RHI access scope.
 * \code{.cpp}
 * template<typename T>
 * auto write(flow::Resource<T> resource, rhi::AccessScope scope);
 * \endcode
 *
 * \param resource Resource written by an RDG operation.
 * \param scope RHI scope in which the write occurs.
 * \return Scoped Flow write descriptor.
 */
template <typename T>
auto write(flow::Resource<T> resource, rhi::AccessScope scope) {
  auto access = flow::write(resource);
  return ScopedAccess<decltype(access)>(std::move(access), scope);
}

/**
 * \brief Records a read-write access with an RHI access scope.
 * \code{.cpp}
 * template<typename T>
 * auto read_write(flow::Resource<T> resource, rhi::AccessScope scope);
 * \endcode
 *
 * \param resource Resource read and written by an RDG operation.
 * \param scope RHI scope in which the access occurs.
 * \return Scoped Flow read-write descriptor.
 */
template <typename T>
auto read_write(flow::Resource<T> resource, rhi::AccessScope scope) {
  auto access = flow::read_write(resource);
  return ScopedAccess<decltype(access)>(std::move(access), scope);
}

} // namespace strobe::rdg
