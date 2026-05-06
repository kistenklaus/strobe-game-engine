#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/ecs/event_queue.hpp"
#include <concepts>
#include <type_traits>
namespace strobe {

namespace ecs {
class EventRegistry;

}

template <typename E> struct Events {
  friend class ecs::EventRegistry;

public:
  using value_type = std::remove_cv_t<E>;
  using access_type = E;
  static constexpr bool is_read_stream = std::is_const_v<E>;
  static constexpr bool is_write_stream = !is_read_stream;

  template <typename U>
    requires(is_write_stream && std::constructible_from<value_type, U &&>)
  void emit(U &&event) {
    m_queue->emplace(std::forward<U>(event));
  }

  span<const value_type> events() const
    requires(is_read_stream)
  {
    return m_queue->span();
  }

private:
  explicit Events(ecs::EventQueue<E> *queue) : m_queue(queue) {}
  ecs::EventQueue<E> *m_queue;
};

namespace ecs::details {
template <typename T> struct IsEvents : std::false_type {};
template <typename R> struct IsEvents<Events<R>> : std::true_type {};
} // namespace ecs::details

template <typename T>
concept events = ecs::details::IsEvents<std::remove_cvref_t<T>>::value;

} // namespace strobe
