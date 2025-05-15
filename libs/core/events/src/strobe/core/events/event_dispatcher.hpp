#pragma once

#include <strobe/core/containers/small_vector.hpp>

#include "event.hpp"
#include "event_dispatcher_interface.hpp"
#include "event_listener.hpp"
#include "event_listener_handle.hpp"

namespace strobe {

template <events::Event E>
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
    for (const auto& l : m_compactListeners) {
      if constexpr (e.canceled()) {
        break;
      }
      l(e);
    }
  }

  EventListenerHandle addListener(EventListenerRef<E> listener) {
    std::uint32_t id;
    if (m_freeIds.empty()) {
      id = m_freeIds.top();
      m_freeIds.pop();
      m_compactListeners[id] = listener;
    } else {
      id = m_listenerIdMap.size();
      m_compactListeners.push_back(listener);
    
    }

    return this->makeHandle(id, this,
                            &EventDispatcher::removeListenerInternally);
  }

  bool removeListener(EventListenerHandle handle) {
    removeListenerInternally(this->detachHandle(handle));
  }

 private:
  bool removeListenerInternally(std::uint32_t id) {}

  strobe::SmallVector<EventListenerRef<E>, 1> m_compactListeners;
  strobe::SmallVector<std::uint32_t, 8> m_listenerIdMap;
  strobe::SmallVector<std::uint32_t, 4> m_freeIds;
};

}  // namespace strobe
