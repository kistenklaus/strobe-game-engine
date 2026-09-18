#pragma once

#include "event_listener_handle.hpp"

namespace strobe::events::details {

class IEventDispatcher {
protected:
  using State = EventDispatcherState;
  using ListenerId = State::ListenerId;

  static EventListenerHandle make_handle(State *state,
                                         ListenerId id) noexcept {
    return EventListenerHandle(state, id);
  }

  static bool release_handle(EventListenerHandle &handle,
                             State *expectedState) noexcept {
    return handle.release_from(expectedState);
  }
};

} // namespace strobe::events::details
