#pragma once

#include <algorithm>
#include <iterator>
#include <print>
#include <strobe/core/containers/small_vector.hpp>

#include "event.hpp"
#include "event_dispatcher_interface.hpp"
#include "event_listener.hpp"
#include "event_listener_handle.hpp"

namespace strobe {

template <events::Event E, Allocator A>
class EventDispatcher : protected events::details::IEventDispatcher {
 private:
  using Self = EventDispatcher<E, A>;
  using Container = SmallVector<EventListenerRef<E>, A>;
  using iterator = Container::iterator;
  using size_type = Container::size_type;

 public:
  using payload_type = E::payload_type;

  EventDispatcher(A alloc = {}) : m_listeners(alloc) { m_dispatchIdx = -1; }

  EventDispatcher(const EventDispatcher&) = delete;
  EventDispatcher& operator=(const EventDispatcher&) = delete;
  EventDispatcher(EventDispatcher&&) = delete;
  EventDispatcher& operator=(EventDispatcher&&) = delete;

  void dispatch(payload_type v) {
    E event(std::move(v));
    dispatch(event);
  }

  void dispatch(E& e) {
    for (m_dispatchIdx = 0; m_dispatchIdx < m_listeners.size();
         m_dispatchIdx++) {
      if (e.canceled()) {
        break;
      }
      m_listeners[m_dispatchIdx](e);
    }
    m_dispatchIdx = -1;
  }

  EventListenerHandle addListener(const EventListenerRef<E>& listener) {
    assert((std::ranges::find(m_listeners, listener) == m_listeners.end()) &&
           "Attempted to register the same listener twice.");
    m_listeners.push_back(listener);
    return this->makeHandle<E>(
        listener, reinterpret_cast<void*>(this),
        [](void* userData, events::EventListenerId id) {
          reinterpret_cast<Self*>(userData)->removeListener(id);
        });
  }

  bool removeListener(EventListenerHandle& handle) {
    return removeListener(handle.detach());
  }

  bool removeListener(const events::EventListenerId& id) {
    auto it = std::ranges::find_if(m_listeners,
                                   [&](const auto& ref) { return ref == id; });

    if (it == m_listeners.end()) {
      return false;
    }

    std::ptrdiff_t idx = std::distance(m_listeners.begin(), it);
    if (idx == m_dispatchIdx) {
      m_listeners[idx] = std::move(m_listeners.back());
      m_listeners.pop_back();  // <- must not modify the origin
      idx--;  // may underflow, but unsigned underflow is not UB.
    } else if (idx > m_dispatchIdx) {
      m_listeners[idx] = std::move(m_listeners.back());
      m_listeners.pop_back();  // <- must not modify the origin
    } else {
      m_listeners[idx] = std::move(m_listeners[m_dispatchIdx]);
      m_listeners[m_dispatchIdx] = std::move(m_listeners.back());
      m_dispatchIdx--;
    }


    return true;
  }

  bool empty() const { return m_listeners.empty(); }

 public:
  Container m_listeners;
  ptrdiff_t m_dispatchIdx;
};

template <events::Event E, Allocator A>
using SharedEventDispatcher = SharedBlock<EventDispatcher<E, A>, A>;

}  // namespace strobe
