#pragma once

#include <print>
#include <utility>

#include "strobe/core/events/event_listener.hpp"
namespace strobe {

namespace events::details {
class IEventDispatcher;
};

class EventListenerHandle {
 public:
  friend events::details::IEventDispatcher;
  using UnregisterCallback = void (*)(void* userData,
                                      events::EventListenerId id);

  EventListenerHandle()
      : m_id(), m_unregisterUserData(nullptr), m_unregisterCallback(nullptr) {}

  ~EventListenerHandle() { release(); }

  EventListenerHandle(const EventListenerHandle&) = delete;
  EventListenerHandle& operator=(const EventListenerHandle&) = delete;

  EventListenerHandle(EventListenerHandle&& o)
      : m_id(o.m_id),
        m_unregisterUserData(std::exchange(o.m_unregisterUserData, nullptr)),
        m_unregisterCallback(std::exchange(o.m_unregisterCallback, nullptr)) {}

  EventListenerHandle& operator=(EventListenerHandle&& o) {
    if (this == &o) {
      return *this;
    }
    release();
    m_id = o.m_id;
    m_unregisterUserData = std::exchange(o.m_unregisterUserData, nullptr);
    m_unregisterCallback = std::exchange(o.m_unregisterCallback, nullptr);
    return *this;
  }

  events::EventListenerId detach() {
    m_unregisterUserData = nullptr;
    m_unregisterCallback = nullptr;
    return std::move(m_id);
  }

  void release() {
    if (m_unregisterCallback != nullptr) {
      m_unregisterCallback(m_unregisterUserData, m_id);
      m_unregisterCallback = nullptr;
      m_unregisterUserData = nullptr;
      m_id = {};
    }
  }

  // NOTE: this function will deferr the destruction (unregister/release) to
  // another object. A deferred callback MUST call
  // eventDispatcher.removeListener directly with the handle!
  void deferr(void* deferredUserData, UnregisterCallback deferredCallback) {
    m_unregisterUserData = deferredUserData;
    m_unregisterCallback = deferredCallback;
  }

 public:
  explicit EventListenerHandle(events::EventListenerId id,
                               void* unregisterUserData,
                               UnregisterCallback unregisterCallback)
      : m_id(id),
        m_unregisterUserData(unregisterUserData),
        m_unregisterCallback(unregisterCallback) {}

  events::EventListenerId m_id;
  void* m_unregisterUserData;
  UnregisterCallback m_unregisterCallback;
};

}  // namespace strobe
