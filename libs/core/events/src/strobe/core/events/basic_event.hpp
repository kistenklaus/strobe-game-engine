#pragma once

#include "event.hpp"
namespace strobe {

template <typename T>
class BasicEvent {
 public:
  explicit BasicEvent(T&& v) : m_payload(std::move(v)) {}
  explicit BasicEvent(const T& v) : m_payload(v) {}

  using payload_type = T;

  [[no_unique_address]] T m_payload;

  const T& payload() const { return m_payload; }

  constexpr bool canceled() const { return false; }
};
static_assert(events::Event<BasicEvent<int>>);

}  // namespace strobe
