#pragma once

#include "event.hpp"
#include "event_dispatcher_interface.hpp"
#include "event_listener.hpp"
#include "event_listener_handle.hpp"
#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Synchronous, layered event dispatcher.
 * \code{.cpp}
 * template<Event E, Allocator A = Mallocator>
 * class EventDispatcher;
 * \endcode
 *
 * Listeners run in ascending layer order. Cancellation is checked between
 * layers.
 * Listeners added during a dispatch are deferred until the outermost dispatch
 * completes.
 *
 * The dispatcher serializes dispatch, registration, removal, and inspection
 * with an internal mutex. Listener handles may be released concurrently with
 * dispatch. The dispatcher object itself must remain alive while operations
 * are in progress; its lifetime must be externally synchronized.
 *
 * \attention 1. Event listeners are non-owning and must outlive dispatch.
 * \attention 2. Destroying the dispatcher from one of its callbacks is not
 * permitted.
 */
template <events::Event E, Allocator A = strobe::Mallocator>
class EventDispatcher : protected events::details::IEventDispatcher {
private:
  using StateBase = events::details::EventDispatcherState;
  using ListenerId = StateBase::ListenerId;
  using AllocatorTraits = strobe::AllocatorTraits<A>;

  struct Entry {
    ListenerId id;
    EventListenerRef<E> listener;
    std::uint8_t layer;
    bool connected;
  };

  using Container = SmallVector<Entry, 4, A>;

  struct State final : StateBase {
    explicit State(const A &allocator)
        : StateBase(&State::disconnect_state, &State::destroy_state),
          allocator(allocator), listeners(allocator) {}

    static bool disconnect_state(StateBase *base, ListenerId id) noexcept {
      return static_cast<State *>(base)->disconnect(id);
    }

    static void destroy_state(StateBase *base) noexcept {
      auto *state = static_cast<State *>(base);

      // The allocator must survive destruction of State because it is used
      // to deallocate the memory containing State.
      A allocator = std::move(state->allocator);

      std::destroy_at(state);
      AllocatorTraits::template deallocate<State>(allocator, state);
    }

    bool disconnect(ListenerId id) noexcept {
      auto it =
          std::find_if(listeners.begin(), listeners.end(),
                       [id](const Entry &entry) { return entry.id == id; });

      if (it == listeners.end() || !it->connected) {
        return false;
      }

      // Do not invalidate an active dispatch iterator. The disconnected
      // entry is removed after the outermost dispatch finishes.
      it->connected = false;
      dirty = true;

      return true;
    }

    void finish_dispatch() noexcept {
      assert(dispatchDepth != 0);
      --dispatchDepth;

      if (dispatchDepth != 0 || !dirty) {
        return;
      }

      auto newEnd =
          std::remove_if(listeners.begin(), listeners.end(),
                         [](const Entry &entry) { return !entry.connected; });

      listeners.erase(newEnd, listeners.end());

      std::sort(listeners.begin(), listeners.end(),
                [](const Entry &lhs, const Entry &rhs) {
                  return lhs.layer < rhs.layer;
                });

      activeListenerCount = listeners.size();
      dirty = false;
    }

    [[no_unique_address]] A allocator;
    Container listeners;

    ListenerId nextId = 1;

    // Entries in [0, activeListenerCount) are visible to dispatch().
    // Listeners added during dispatch are appended outside this prefix.
    std::size_t activeListenerCount = 0;

    std::size_t dispatchDepth = 0;
    bool dirty = false;
  };

  struct DispatchScope {
    State *state;

    ~DispatchScope() noexcept { state->finish_dispatch(); }
  };

public:
  /**
   * \brief Type of the event payload.
   * \code{.cpp}
   * using payload_type = E::payload_type;
   * \endcode
   */
  using payload_type = E::payload_type;

  /**
   * \brief Type used to order listener layers.
   * \code{.cpp}
   * using layer_type = std::uint8_t;
   * \endcode
   */
  using layer_type = std::uint8_t;

  /**
   * \brief Constructs and destroys a dispatcher.
   * \code{.cpp}
   * explicit EventDispatcher(A allocator = {});
   * EventDispatcher(const EventDispatcher&) = delete;
   * EventDispatcher& operator=(const EventDispatcher&) = delete;
   * EventDispatcher(EventDispatcher&&) = delete;
   * EventDispatcher& operator=(EventDispatcher&&) = delete;
   * ~EventDispatcher() noexcept;
   * \endcode
   *
   * The dispatcher initially has no listeners. Destroying it disconnects all
   * listeners owned by the dispatcher; outstanding handles become inert.
   *
   * \param allocator Allocator used for dispatcher state and listener storage.
   * \attention 1. The dispatcher must outlive every operation performed on it.
   * \attention 2. Destruction must not occur from one of the dispatcher's
   * callbacks.
   */
  explicit EventDispatcher(A allocator = {}) : m_state(make_state(allocator)) {}

  EventDispatcher(const EventDispatcher &) = delete;
  EventDispatcher &operator=(const EventDispatcher &) = delete;

  EventDispatcher(EventDispatcher &&) = delete;
  EventDispatcher &operator=(EventDispatcher &&) = delete;

  ~EventDispatcher() noexcept {
    State *state = std::exchange(m_state, nullptr);

    if (state == nullptr) {
      return;
    }

    {
      std::lock_guard lock{state->mutex};

      // Destroying the dispatcher from one of its own callbacks would
      // destroy the object whose dispatch() function is still executing.
      assert(state->dispatchDepth == 0);

      state->alive = false;
      state->listeners.clear();
      state->activeListenerCount = 0;
      state->dirty = false;
    }

    // Outstanding listener handles keep State alive, but it is marked dead
    // and those handles become inert.
    state->release_reference();
  }

  /**
   * \brief Dispatches an event synchronously.
   * \code{.cpp}
   * void dispatch(payload_type payload);
   * void dispatch(E& event);
   * \endcode
   *
   * The payload overload constructs an event and dispatches it. The event
   * overload delivers the event to connected listeners in ascending layer
   * order. Cancellation is observed between layers, and listeners added while
   * dispatching are deferred until the outermost dispatch completes.
   *
   * \param payload Payload to move into the event.
   * \param event Event to deliver to listeners.
   * \attention 1. The event object must remain valid for the duration of the
   * dispatch.
   */
  void dispatch(payload_type payload) {
    E event(std::move(payload));
    dispatch(event);
  }

  void dispatch(E &event) {
    State *state = m_state;
    assert(state != nullptr);

    // The mutex is intentionally held across listener invocation. This keeps
    // dispatch free of per-listener atomic operations and makes destruction
    // of handles on other threads wait for callbacks to finish.
    std::lock_guard lock{state->mutex};
    assert(state->alive);

    if (event.canceled()) {
      return;
    }

    ++state->dispatchDepth;
    DispatchScope scope{state};

    // Do not include listeners added during this dispatch.
    const std::size_t end = state->activeListenerCount;
    std::size_t index = 0;

    while (index < end) {
      const layer_type layer = state->listeners[index].layer;

      do {
        // Copy the non-owning listener reference because the callback may
        // append a listener and cause the container to reallocate.
        const bool connected = state->listeners[index].connected;

        const EventListenerRef<E> listener = state->listeners[index].listener;

        ++index;

        if (connected) {
          listener(event);
        }
      } while (index < end && state->listeners[index].layer == layer);

      // Cancellation applies between layers. All connected listeners in the
      // current layer receive the event.
      if (event.canceled()) {
        break;
      }
    }
  }

  /**
   * \brief Registers a listener in a dispatch layer.
   * \code{.cpp}
   * EventListenerHandle addListener(const EventListenerRef<E>& listener,
   *                                 layer_type layer = 0);
   * \endcode
   *
   * \param listener Non-owning callback reference.
   * \param layer Layer used for ordering and cancellation boundaries.
   * \return RAII handle for the registration.
   */
  EventListenerHandle addListener(const EventListenerRef<E> &listener,
                                  layer_type layer = 0) {
    State *state = m_state;
    assert(state != nullptr);

    std::lock_guard lock{state->mutex};
    assert(state->alive);

    const ListenerId id = state->nextId++;
    assert(id != 0 && "Event listener ID overflowed.");

    const Entry entry{
        .id = id,
        .listener = listener,
        .layer = layer,
        .connected = true,
    };

    if (state->dispatchDepth == 0) {
      auto position =
          std::upper_bound(state->listeners.begin(), state->listeners.end(),
                           layer, [](layer_type value, const Entry &candidate) {
                             return value < candidate.layer;
                           });

      state->listeners.insert(position, entry);
      ++state->activeListenerCount;
    } else {
      // The new listener remains invisible until the outermost dispatch has
      // completed, including to nested dispatches.
      state->listeners.push_back(entry);
      state->dirty = true;
    }

    return make_handle(state, id);
  }

  /**
   * \brief Releases a listener handle owned by this dispatcher.
   * \code{.cpp}
   * bool removeListener(EventListenerHandle& handle) noexcept;
   * \endcode
   *
   * \param handle Handle returned by addListener.
   * \return Whether the listener was connected.
   * \attention 1. A handle owned by this dispatcher is cleared even when its
   * listener was already disconnected.
   */
  bool removeListener(EventListenerHandle &handle) noexcept {
    return release_handle(handle, m_state);
  }

  /**
   * \brief Returns whether no listeners are currently connected.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   */
  [[nodiscard]] bool empty() const noexcept {
    State *state = m_state;
    assert(state != nullptr);

    std::lock_guard lock{state->mutex};

    return std::none_of(state->listeners.begin(), state->listeners.end(),
                        [](const Entry &entry) { return entry.connected; });
  }

private:
  static State *make_state(A &allocator) {
    State *state = AllocatorTraits::template allocate<State>(allocator);

    assert(state != nullptr);

    return std::construct_at(state, allocator);
  }

  State *m_state;
};

} // namespace strobe
