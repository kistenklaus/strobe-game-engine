#pragma once

#include <utility>
namespace strobe {

namespace events::details {
class IEventDispatcher;
};

class EventListenerHandle {
 public:
  friend events::details::IEventDispatcher;
  using UnregisterCallback = void (*)(void* userData, std::size_t id);

  EventListenerHandle()
      : m_id(0), m_unregisterUserData(nullptr), m_unregisterCallback(nullptr) {}

  ~EventListenerHandle() {}

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
    m_unregisterUserData = std::exchange(o.m_unregisterUserData, nullptr);
    m_unregisterCallback = std::exchange(o.m_unregisterCallback, nullptr);
    return *this;
  }

  void detach() {
    m_unregisterUserData = nullptr;
    m_unregisterCallback = nullptr;
  }

  void release() {
    if (m_unregisterCallback != nullptr) {
      m_unregisterCallback(m_unregisterUserData, m_id);
      m_unregisterCallback = nullptr;
      m_unregisterUserData = nullptr;
    }
  }

 private:
  explicit EventListenerHandle(std::uint32_t id, void* unregisterUserData,
                               UnregisterCallback unregisterCallback) {}

  std::uint32_t m_id;
  void* m_unregisterUserData;
  UnregisterCallback m_unregisterCallback;
};

}  // namespace strobe
