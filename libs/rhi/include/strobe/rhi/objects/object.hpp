#pragma once

namespace strobe::rhi {

template <typename Derived> class Object {
protected:
  explicit Object(void *handle) noexcept : m_handle(handle) {
    static_assert(sizeof(Derived) == sizeof(void *));
    static_assert(alignof(Derived) == alignof(void *));
  }
  Object(const Object &) noexcept;
  void *m_handle;
};

} // namespace strobe::rhi
