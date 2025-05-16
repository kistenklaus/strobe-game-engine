#pragma once

#include <strobe/core/containers/small_vector.hpp>

#include "event.hpp"
#include "event_dispatcher_interface.hpp"
#include "event_listener.hpp"
#include "event_listener_handle.hpp"
#include "strobe/core/containers/linear_set.hpp"

namespace strobe {

template <events::Event E, Allocator A>
class EventDispatcher : protected events::details::IEventDispatcher {
 public:
  using payload_type = E::payload_type;

  EventDispatcher() {}

  EventDispatcher(const EventDispatcher&) = delete;
  EventDispatcher& operator=(const EventDispatcher&) = delete;
  EventDispatcher(EventDispatcher&&) = delete;
  EventDispatcher& operator=(EventDispatcher&&) = delete;

  void dispatch(payload_type v) { dispatch(E(std::move(v))); }

  void dispatch(const E& e) {
  }

  EventListenerHandle addListener(EventListenerRef<E> listener) {
    std::uint32_t id = 0;

    return this->makeHandle(id, this,
                            &EventDispatcher::removeListenerInternally);
  }

  bool removeListener(EventListenerHandle handle) {
    removeListenerInternally(this->detachHandle(handle));
  }

 private:
  bool removeListenerInternally(std::uint32_t id) {}
  

  LinearSet<EventListenerRef<E>, A> m_listeners;
};

}  // namespace strobe
