#pragma once

#include "event_listener_handle.hpp"
namespace strobe::events::details {

class IEventDispatcher {
 protected:
  EventListenerHandle makeHandle(
      std::uint32_t id, void* userData,
      EventListenerHandle::UnregisterCallback unregisterCallback) {
    return EventListenerHandle(id, userData, unregisterCallback);
  }

  std::size_t detachHandle(EventListenerHandle handle) {
    std::size_t id = handle.m_id;
    handle.detach();
    return id;
  }
};

}  // namespace strobe::events::details
