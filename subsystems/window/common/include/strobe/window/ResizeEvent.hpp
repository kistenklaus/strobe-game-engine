#pragma once

#include <strobe/lina.hpp>

namespace strobe::window {

class ResizeEvent {
 public:
  using payload_type = uvec2;

  explicit ResizeEvent(payload_type&& v) : m_payload(std::move(v)) {}
  explicit ResizeEvent(const payload_type& v) : m_payload(v) {}

  [[no_unique_address]] payload_type m_payload;

  const payload_type& payload() const { return m_payload; }

  constexpr bool canceled() const { return false; }
};

}
