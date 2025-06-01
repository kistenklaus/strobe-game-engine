#pragma once

#include "strobe/window/Action.hpp"
#include "strobe/window/Key.hpp"
#include "strobe/window/Mod.hpp"


namespace strobe::window {

class KeyboardEvent {
 public:
  struct Payload {
    Key key;
    Action action;
    Mod mod;
  };

  explicit KeyboardEvent(Payload&& v) : m_payload(std::move(v)) {}
  explicit KeyboardEvent(const Payload& v) : m_payload(v) {}

  using payload_type = Payload;

  const Payload& payload() const { return m_payload; }

  constexpr bool canceled() const { return false; }

 private:
  [[no_unique_address]] Payload m_payload;
};


}
