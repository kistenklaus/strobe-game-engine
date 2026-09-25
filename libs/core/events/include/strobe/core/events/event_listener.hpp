#pragma once

#include "event.hpp"

#include <cassert>

namespace strobe {

/**
 * \ingroup core
 * \brief Native callback signature used by event listeners.
 * \code{.cpp}
 * template<Event E>
 * using EventCallback = void (*)(void* userData, const E& event);
 * \endcode
 */
template <events::Event E>
using EventCallback = void (*)(void *userData, const E &event);

namespace events {

/**
 * \brief Constraint for callable objects accepted by EventListenerRef.
 * \code{.cpp}
 * template<typename L, typename E>
 * concept EventCallableListener;
 * \endcode
 *
 * The callable must accept a const reference to the event type.
 */
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

/**
 * \ingroup core
 * \brief Non-owning reference to an event callback.
 * \code{.cpp}
 * template<Event E>
 * class EventListenerRef;
 * \endcode
 *
 * The referenced callable and its user data must outlive every dispatch that
 * can invoke this reference.
 */
template <events::Event E> class EventListenerRef {
public:
  /**
   * \brief Type of the referenced event payload.
   * \code{.cpp}
   * using payload_type = E::payload_type;
   * \endcode
   */
  using payload_type = E::payload_type;

  /**
   * \brief Creates a listener from a native callback and user data.
   * \code{.cpp}
   * static EventListenerRef fromNative(void* userData,
   *                                    EventCallback<E> callback);
   * \endcode
   *
   * \param userData Pointer passed to callback.
   * \param callback Native callback to invoke.
   * \return A non-owning listener reference.
   * \attention 1. \p callback must not be null.
   * \attention 2. \p userData must remain valid while the listener can be
   * invoked.
   */
  static EventListenerRef fromNative(void *userData,
                                     EventCallback<E> callback) {
    return EventListenerRef(userData, callback);
  }

  /**
   * \brief Creates a listener from a callable object pointer.
   * \code{.cpp}
   * template<EventCallableListener<E> L>
   * static EventListenerRef fromCallable(L* callable);
   * \endcode
   *
   * \param callable Callable object to invoke.
   * \return A non-owning listener reference.
   * \attention 1. \p callable must not be null.
   * \attention 2. \p callable must remain valid while the listener can be
   * invoked.
   */
  template <events::EventCallableListener<E> L>
  static EventListenerRef fromCallable(L *callable) {
    return EventListenerRef(callable, [](void *userData, const E &event) {
      (*static_cast<L *>(userData))(event);
    });
  }

  /**
   * \brief Creates a listener from a non-const member function.
   * \code{.cpp}
   * template<typename M, void (M::*MemberFunction)(const E&)>
   * static EventListenerRef fromMemberFunction(M* self);
   * \endcode
   *
   * \param self Object on which the member function is invoked.
   * \return A non-owning listener reference.
   * \attention 1. \p self must not be null.
   * \attention 2. \p self must remain valid while the listener can be
   * invoked.
   */
  template <typename M, void (M::*MemberFunction)(const E &)>
  static EventListenerRef fromMemberFunction(M *self) {
    return EventListenerRef(
        self, events::details::bind_member_function<E, M, MemberFunction>);
  }

  /**
   * \brief Invokes the referenced callback.
   * \code{.cpp}
   * void operator()(const E& event) const;
   * \endcode
   *
   * \param event Event passed to the referenced callback.
   * \attention 1. The callback and its user data must remain valid for the
   * duration of the call.
   */
  void operator()(const E &event) const {
    assert(m_callback != nullptr);
    m_callback(m_userData, event);
  }

  /**
   * \brief Compares two listener references.
   * \code{.cpp}
   * bool operator==(const EventListenerRef& other) const noexcept;
   * \endcode
   *
   * \param other Listener reference to compare.
   * \return Whether both references contain the same callback and user data.
   */
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
