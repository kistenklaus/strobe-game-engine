#pragma once

#include "strobe/core/events/event_listener.hpp"
#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/platform/monitor.hpp"
#include <tuple>

namespace strobe::platform {

class MonitorEvent {
public:
  using payload_type = std::tuple<Monitor, bool>;

  explicit MonitorEvent(payload_type payload) noexcept
      : m_payload(std::move(payload)) {}

  [[nodiscard]] bool canceled() const noexcept { return false; }

  [[nodiscard]] const payload_type &payload() const noexcept {
    return m_payload;
  }

  [[nodiscard]] bool connect() const noexcept {
    return std::get<bool>(m_payload);
  }

  [[nodiscard]] bool disconnect() const noexcept { return !connect(); }

  [[nodiscard]] const Monitor &monitor() const noexcept {
    return std::get<Monitor>(m_payload);
  }

private:
  payload_type m_payload;
};

[[nodiscard]] EventListenerHandle
add_monitor_listener(const EventListenerRef<MonitorEvent> &listener) noexcept;

} // namespace strobe::platform
