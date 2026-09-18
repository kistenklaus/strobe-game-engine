#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <utility>

namespace strobe {

namespace events::details {

struct EventDispatcherState {
  using ListenerId = std::uint64_t;

  using DisconnectFunction = bool (*)(EventDispatcherState *,
                                      ListenerId) noexcept;

  using DestroyFunction = void (*)(EventDispatcherState *) noexcept;

  EventDispatcherState(DisconnectFunction disconnect,
                       DestroyFunction destroy) noexcept
      : disconnect(disconnect), destroy(destroy) {}

  EventDispatcherState(const EventDispatcherState &) = delete;
  EventDispatcherState &operator=(const EventDispatcherState &) = delete;

  void add_reference() noexcept {
    references.fetch_add(1, std::memory_order_relaxed);
  }

  void release_reference() noexcept {
    if (references.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      destroy(this);
    }
  }

  std::atomic<std::size_t> references{1};

  // Recursive so a callback may unregister itself or perform a nested
  // dispatch through the same dispatcher.
  std::recursive_mutex mutex;

  bool alive = true;

  DisconnectFunction disconnect;
  DestroyFunction destroy;
};

class IEventDispatcher;

} // namespace events::details

class EventListenerHandle {
public:
  EventListenerHandle() noexcept = default;

  ~EventListenerHandle() noexcept { release(); }

  EventListenerHandle(const EventListenerHandle &) = delete;
  EventListenerHandle &operator=(const EventListenerHandle &) = delete;

  EventListenerHandle(EventListenerHandle &&other) noexcept
      : m_state(std::exchange(other.m_state, nullptr)),
        m_id(std::exchange(other.m_id, 0)) {}

  EventListenerHandle &operator=(EventListenerHandle &&other) noexcept {
    if (this == &other) {
      return *this;
    }

    release();

    m_state = std::exchange(other.m_state, nullptr);
    m_id = std::exchange(other.m_id, 0);

    return *this;
  }

  [[nodiscard]] explicit operator bool() const noexcept {
    return m_state != nullptr;
  }

  bool release() noexcept { return release_from(nullptr); }

private:
  friend events::details::IEventDispatcher;

  using State = events::details::EventDispatcherState;
  using ListenerId = State::ListenerId;

  EventListenerHandle(State *state, ListenerId id) noexcept
      : m_state(state), m_id(id) {
    m_state->add_reference();
  }

  bool release_from(State *expectedState) noexcept {
    if (m_state == nullptr ||
        (expectedState != nullptr && m_state != expectedState)) {
      return false;
    }

    State *state = std::exchange(m_state, nullptr);
    const ListenerId id = std::exchange(m_id, 0);

    bool disconnected = false;

    {
      std::lock_guard lock{state->mutex};

      if (state->alive) {
        disconnected = state->disconnect(state, id);
      }
    }

    state->release_reference();

    return disconnected;
  }

  State *m_state = nullptr;
  ListenerId m_id = 0;
};

} // namespace strobe
