#pragma once

#include <utility>
namespace strobe::rhi {

namespace detail {
template <typename T> T make_object(void *handle) noexcept;
}

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

  friend bool operator==(const Derived &lhs, const Derived &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }

  friend bool operator!=(const Derived &lhs, const Derived &rhs) noexcept {
    return !(lhs == rhs);
  }

private:
  template <typename T> friend T detail::make_object(void *) noexcept;

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
