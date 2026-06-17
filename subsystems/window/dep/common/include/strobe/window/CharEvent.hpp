#pragma once

namespace strobe::window {

class CharEvent {
 public:
  using payload_type = char32_t;

  explicit CharEvent(payload_type&& v) : m_payload(std::move(v)) {}
  explicit CharEvent(const payload_type& v) : m_payload(v) {}

  const payload_type& payload() const { return m_payload; }
  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] payload_type m_payload;
};

}
