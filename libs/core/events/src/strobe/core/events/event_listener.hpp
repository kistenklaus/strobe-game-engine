#pragma once

#include "event.hpp"
namespace strobe {

template <events::Event E>
using EventCallback = void (*)(void* userData, const E& e);

namespace events {

template <typename L, typename E>
concept EventCallableListener =
    events::Event<E> &&
    requires(L& listener, const typename E::payload_type& e) {
      { listener(e) };
    };

}  // namespace events

template <events::Event E>
class EventListenerRef {
 public:
  using payload_type = E::payload_type;

  static EventListenerRef fromNative(void* userData,
                                     EventCallback<E> callback) {
    return EventListenerRef<E>(userData, callback);
  }

  template <events::EventCallableListener<E> L>
  static EventListenerRef fromCallable(L* callable) {
    return EventListenerRef<E>(callable, [](void* userData, const E& e) {
      (*reinterpret_cast<L*>(userData))(e);
    });
  }

  void operator()(const E& e) const {
    assert(m_callback != nullptr);
    m_callback(m_userData, e);
  }

 private:
  explicit EventListenerRef(void* userData, EventCallback<E> callback)
      : m_userData(userData), m_callback(callback) {
    assert(m_callback != nullptr);
  }

  const void* m_userData;
  const EventCallback<E> m_callback;
};

}  // namespace strobe
