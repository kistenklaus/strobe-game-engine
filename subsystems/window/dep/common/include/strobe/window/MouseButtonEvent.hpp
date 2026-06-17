#pragma once

#include <utility>
#include "strobe/window/Action.hpp"
#include "strobe/window/Mod.hpp"
#include "strobe/window/MouseButton.hpp"
namespace strobe::window {

class MouseButtonEvent {
 public:
  struct Payload {
    MouseButton button;
    Action action;
    Mod mod;
  };

  explicit MouseButtonEvent(Payload&& v) : m_payload(std::move(v)) {}
  explicit MouseButtonEvent(const Payload& v) : m_payload(v) {}

  using payload_type = Payload;

  const Payload& payload() const { return m_payload; }

  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] Payload m_payload;
};

}
