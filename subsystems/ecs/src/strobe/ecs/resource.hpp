#pragma once

#include <type_traits>
namespace strobe {

template <typename R> struct Resource {
public:
  using value_type = std::remove_cv_t<R>;
  using access_type = R;
  static constexpr bool is_read_only = std::is_const_v<R>;
  static constexpr bool is_writable = !is_read_only;

  Resource() = default;
  explicit Resource(value_type *ptr) : m_ptr(ptr) {}

  R &get() const { return *m_ptr; }

  R *operator->() const { return m_ptr; }

  R &operator*() const { return *m_ptr; }

private:
  value_type *m_ptr;
};

namespace ecs::details {
template <typename T> struct IsResource : std::false_type {};
template <typename R> struct IsResource<Resource<R>> : std::true_type {};
} // namespace ecs::details

template <typename T>
concept resource = ecs::details::IsResource<std::remove_cvref_t<T>>::value;

} // namespace strobe
