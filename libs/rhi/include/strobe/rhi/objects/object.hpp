#pragma once

#include <utility>

namespace strobe::rhi {

namespace detail {
template <typename T> T make_object(void *handle) noexcept;
}

/**
 * \ingroup rhi
 * \brief Device object handle.
 * Defined in header <strobe/rhi/objects/object.hpp>
 * \code{.cpp}
 * template<typename Derived>
 * class Object;
 * \endcode
 *
 * Common reference-counted handle implementation for RHI objects.
 *
 * A default-constructed or moved-from Object does not reference an object.
 * Copies share ownership of the referenced object, while moves transfer
 * ownership without changing its reference count.
 *
 * \tparam Derived Concrete RHI object type.
 */
template <typename Derived> class Object {
public:
  Object() noexcept = default;
  Object(const Object &o) noexcept : m_handle(o.m_handle) {
    if (m_handle != nullptr) {
      Derived::pin(m_handle);
    }
  }
  Object(Object &&o) noexcept : m_handle(std::exchange(o.m_handle, nullptr)) {}

  Object &operator=(const Object &o) noexcept {
    if (this == &o)
      return *this;

    if (o.m_handle != nullptr) {
      Derived::pin(o.m_handle);
    }
    Derived::unpin(m_handle);

    m_handle = o.m_handle;
    return *this;
  }

  Object &operator=(Object &&o) noexcept {
    if (this == &o)
      return *this;

    if (m_handle != nullptr) {
      Derived::unpin(m_handle);
    }

    m_handle = std::exchange(o.m_handle, nullptr);
    return *this;
  }

  ~Object() noexcept {
    if (m_handle)
      Derived::unpin(m_handle);
  }

  explicit operator bool() const noexcept { return m_handle != nullptr; }

  /**
   * \brief Compares object handles.
   *
   * Tests whether two objects reference the same underlying RHI object.
   *
   * \param lhs Left-hand object.
   * \param rhs Right-hand object.
   *
   * \return true if both objects reference the same object.
   */
  friend bool operator==(const Derived &lhs, const Derived &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }

  /**
   * \brief Compares object handles.
   *
   * Tests whether two objects reference different underlying RHI objects.
   *
   * \param lhs Left-hand object.
   * \param rhs Right-hand object.
   *
   * \return true if the objects reference different objects.
   */
  friend bool operator!=(const Derived &lhs, const Derived &rhs) noexcept {
    return !(lhs == rhs);
  }

private:
  template <typename T> friend T detail::make_object(void *) noexcept;

  /**
   * \brief Wraps internal handle.
   *
   * Constructs a derived RHI object from an owned internal handle.
   *
   * \param handle Internal object handle.
   *
   * \return RHI object owning \p handle.
   *
   * \attention 1. \p handle must reference a valid implementation of
   * \p Derived.
   * \attention 2. Ownership of \p handle is transferred to the returned object.
   */
  static Derived from_handle(void *handle) noexcept {
    Derived object;
    object.m_handle = handle;

    static_assert(sizeof(Derived) == sizeof(void *));
    static_assert(alignof(Derived) == alignof(void *));

    return object;
  }

protected:
  explicit Object(void *handle) noexcept : m_handle(handle) {
    static_assert(sizeof(Derived) == sizeof(void *));
    static_assert(alignof(Derived) == alignof(void *));
  }

  void *m_handle = nullptr;
};

} // namespace strobe::rhi
