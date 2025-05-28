#pragma once

#include "event_listener_handle.hpp"
namespace strobe::events::details {

class IEventDispatcher {
 protected:
  EventListenerHandle makeHandle(
      EventListenerId id, void* userData,
      EventListenerHandle::UnregisterCallback unregisterCallback) {
    return EventListenerHandle(id, userData, unregisterCallback);
  }

  template <events::Event E>
  EventListenerHandle makeHandle(
      const EventListenerRef<E>& ref, void* userData,
      EventListenerHandle::UnregisterCallback unregisterCallback) {
    return EventListenerHandle(EventListenerId(ref), userData,
                               unregisterCallback);
  }

  EventListenerId detachHandle(EventListenerHandle& handle) {
    EventListenerId id = handle.m_id;
    handle.detach();
    return id;
  }
};

}  // namespace strobe::events::details
