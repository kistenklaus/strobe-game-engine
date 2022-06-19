#pragma once
#include "strb/core/window/strobe_keys.hpp"
#include "strb/stl.hpp"

namespace strb::window {

class Keyboard {
protected:
  strb::bitset<STROBE_KEY_LAST + 1> keys;

public:
  inline strb::boolean isKeyDown(key_t key) const { return keys[key]; }
};

} // namespace strb::window
