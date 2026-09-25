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

/**
 * \ingroup core
 * \brief Move-only RAII handle for a registered event listener.
 * \code{.cpp}
 * class EventListenerHandle;
 * \endcode
 *
 * Releasing or destroying the handle disconnects its listener. Handles may be
 * released while dispatch is running; removal waits for the dispatcher lock.
 * A handle may outlive its dispatcher, in which case it becomes inert.
 * Operations on the same handle require external synchronization.
 */
class EventListenerHandle {
public:
  /**
   * \brief Constructs, moves, and destroys a listener handle.
   * \code{.cpp}
   * EventListenerHandle() noexcept;
   * ~EventListenerHandle() noexcept;
   * EventListenerHandle(const EventListenerHandle&) = delete;
   * EventListenerHandle& operator=(const EventListenerHandle&) = delete;
   * EventListenerHandle(EventListenerHandle&& other) noexcept;
   * EventListenerHandle& operator=(EventListenerHandle&& other) noexcept;
   * \endcode
   *
   * A default-constructed handle is disconnected. Destroying or overwriting
   * a connected handle releases its listener. Move construction transfers the
   * registration and leaves the source disconnected.
   *
   * \param other Handle whose registration is transferred.
   * \attention 1. The source and destination handles must not be accessed
   * concurrently during a move.
   */
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

  /**
   * \brief Reports whether the handle is connected to dispatcher state.
   * \code{.cpp}
   * explicit operator bool() const noexcept;
   * \endcode
   *
   * \return Whether the handle retains dispatcher state. This remains true
   * after dispatcher destruction until the handle is released or destroyed.
   */
  [[nodiscard]] explicit operator bool() const noexcept {
    return m_state != nullptr;
  }

  /**
   * \brief Disconnects the listener and returns whether it was connected.
   * \code{.cpp}
   * bool release() noexcept;
   * \endcode
   *
   * Releasing an already disconnected handle has no effect. If dispatch is
   * running, this operation waits until it can acquire the dispatcher lock.
   * A handle whose dispatcher has been destroyed is cleared without
   * disconnecting a listener.
   *
   * \return Whether this call disconnected a connected listener.
   */
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
