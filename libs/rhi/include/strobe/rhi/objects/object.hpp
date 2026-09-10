#pragma once

namespace strobe::rhi {

template <typename Derived> class Object {
public:
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  friend bool operator==(const Derived &lhs, const Derived &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }

  friend bool operator!=(const Derived &lhs, const Derived &rhs) noexcept {
    return !(lhs == rhs);
  }

protected:
  explicit Object(void *handle) noexcept : m_handle(handle) {
    static_assert(sizeof(Derived) == sizeof(void *));
    static_assert(alignof(Derived) == alignof(void *));
  }
  Object(const Object &) noexcept;
  void *m_handle;
};

} // namespace strobe::rhi
