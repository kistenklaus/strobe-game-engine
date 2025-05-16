#pragma once

#include <concepts>

#include "basic_event.hpp"
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

class EventListenerId;  // fwd

}  // namespace events

template <events::Event E>
class EventListenerRef {
 public:
  friend events::EventListenerId;
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

  bool operator==(const EventListenerRef& ref) const noexcept {
    return ref.m_userData == m_userData && ref.m_callback == m_callback;
  }
  bool operator!=(const EventListenerRef& ref) const noexcept = default;

 private:
  explicit EventListenerRef(void* userData, EventCallback<E> callback)
      : m_userData(userData), m_callback(callback) {
    assert(m_callback != nullptr);
  }

  const void* m_userData;
  const EventCallback<E> m_callback;
};

namespace events {

class EventListenerId {
 public:
  template <events::Event E>
  EventListenerId(EventListenerRef<E> ref)
      : m_userData(ref.m_userData),
        m_callback(reinterpret_cast<std::uintptr_t>(ref.m_callback)) {}

 private:
  bool operator==(const EventListenerId& o) const noexcept {
    return o.m_userData == m_userData && o.m_callback == m_callback;
  }

  bool operator!=(const EventListenerId& o) const noexcept = default;

  template <events::Event E>
  bool operator==(const EventListenerRef<E>& ref) const noexcept {
    return ref.m_userData == m_userData &&
           reinterpret_cast<std::uintptr_t>(ref.m_callback) == m_callback;
  }
  template <events::Event E>
  bool operator!=(const EventListenerRef<E>& ref) const noexcept {
    return !(*this == ref);
  }

  void* m_userData;
  std::uintptr_t m_callback;
};
}  // namespace events

static_assert(std::equality_comparable<EventListenerRef<BasicEvent<int>>>);
static_assert(std::equality_comparable_with<
              EventListenerRef<BasicEvent<int>>, events::EventListenerId>);

}  // namespace strobe
