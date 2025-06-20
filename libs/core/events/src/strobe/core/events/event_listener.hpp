#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>

#include "basic_event.hpp"
#include "event.hpp"

namespace strobe {

template <events::Event E>
using EventCallback = void (*)(void* userData, const E& e);

namespace events {

template <typename L, typename E>
concept EventCallableListener =
    events::Event<E> && requires(L& listener, const E& e) {
      { listener(e) };
    };

class EventListenerId;  // fwd
                        //
namespace details {

template <events::Event E, typename M, void (M::*MemberFunc)(const E&)>
void bindMemberFunc(void* userData, const E& e) {
  (reinterpret_cast<M*>(userData)->*MemberFunc)(e);
}

}  // namespace details

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

  template <typename M, void (M::*MemberFunc)(const E&)>
  static EventListenerRef fromMemberFunction(M* self) {
    return EventListenerRef<E>(
        self, events::details::bindMemberFunc<E, M, MemberFunc>);
  }

  void operator()(const E& e) const {
    assert(m_callback != nullptr);
    m_callback(m_userData, e);
  }

  // ref == ref
  bool operator==(const EventListenerRef& ref) const noexcept {
    return (ref.m_userData == m_userData) && (ref.m_callback == m_callback);
  }
  bool operator!=(const EventListenerRef& ref) const noexcept = default;

 private:
  explicit EventListenerRef(void* userData, EventCallback<E> callback)
      : m_userData(userData), m_callback(callback) {
    assert(m_callback != nullptr);
  }

  void* m_userData;
  EventCallback<E> m_callback;
};

namespace events {

class EventListenerId {
 public:
  template <events::Event E>
  EventListenerId(EventListenerRef<E> ref)
      : m_userData(ref.m_userData),
        m_callback(reinterpret_cast<std::uintptr_t>(ref.m_callback)) {}

  EventListenerId()
      : m_userData(nullptr),
        m_callback(reinterpret_cast<std::uintptr_t>(nullptr)) {}

  template <events::Event E>
  friend bool operator==(const EventListenerRef<E>& ref,
                         const EventListenerId& id) noexcept;

  template <events::Event E>
  friend bool operator!=(const EventListenerRef<E>& ref,
                         const EventListenerId& id) noexcept;

  bool operator==(const EventListenerId& o) const noexcept {
    return o.m_userData == m_userData && o.m_callback == m_callback;
  }

  bool operator!=(const EventListenerId& o) const noexcept = default;

 public:
  // id == id

  void* m_userData;
  std::uintptr_t m_callback;
};

// ref == id
template <events::Event E>
bool operator==(const EventListenerRef<E>& ref,
                const EventListenerId& id) noexcept {
  return EventListenerId(ref) == id;
}

template <events::Event E>
bool operator!=(const EventListenerRef<E>& ref,
                const EventListenerId& id) noexcept {
  return !(ref == id);
}

template <events::Event E>
bool operator==(const EventListenerId& id,
                const EventListenerRef<E>& ref) noexcept {
  return ref == id;
}

template <events::Event E>
bool operator!=(const EventListenerId& id,
                const EventListenerRef<E>& ref) noexcept {
  return !(ref == id);
}

}  // namespace events

static_assert(std::equality_comparable<EventListenerRef<BasicEvent<int>>>);
static_assert(std::equality_comparable_with<EventListenerRef<BasicEvent<int>>,
                                            events::EventListenerId>);

}  // namespace strobe
