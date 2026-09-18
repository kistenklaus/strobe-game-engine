#pragma once

#include "event.hpp"

#include <cassert>

namespace strobe {

template <events::Event E>
using EventCallback = void (*)(void *userData, const E &event);

namespace events {

template <typename L, typename E>
concept EventCallableListener =
    events::Event<E> &&
    requires(L &listener, const E &event) { listener(event); };

namespace details {

template <events::Event E, typename M, void (M::*MemberFunction)(const E &)>
void bind_member_function(void *userData, const E &event) {
  (static_cast<M *>(userData)->*MemberFunction)(event);
}

} // namespace details

} // namespace events

template <events::Event E> class EventListenerRef {
public:
  using payload_type = E::payload_type;

  static EventListenerRef fromNative(void *userData,
                                     EventCallback<E> callback) {
    return EventListenerRef(userData, callback);
  }

  template <events::EventCallableListener<E> L>
  static EventListenerRef fromCallable(L *callable) {
    return EventListenerRef(callable, [](void *userData, const E &event) {
      (*static_cast<L *>(userData))(event);
    });
  }

  template <typename M, void (M::*MemberFunction)(const E &)>
  static EventListenerRef fromMemberFunction(M *self) {
    return EventListenerRef(
        self, events::details::bind_member_function<E, M, MemberFunction>);
  }

  void operator()(const E &event) const {
    assert(m_callback != nullptr);
    m_callback(m_userData, event);
  }

  bool operator==(const EventListenerRef &) const noexcept = default;

private:
  explicit EventListenerRef(void *userData, EventCallback<E> callback)
      : m_userData(userData), m_callback(callback) {
    assert(m_callback != nullptr);
  }

  void *m_userData;
  EventCallback<E> m_callback;
};

} // namespace strobe
