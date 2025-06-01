#pragma once

#include <strobe/lina.hpp>

namespace strobe::window {

class MouseMoveEvent {
 public:
  using payload_type = vec2;

  explicit MouseMoveEvent(payload_type&& v) : m_payload(std::move(v)) {}
  explicit MouseMoveEvent(const payload_type& v) : m_payload(v) {}

  const payload_type& payload() const { return m_payload; }
  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] payload_type m_payload;
};

}
