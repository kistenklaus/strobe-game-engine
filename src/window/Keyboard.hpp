#pragma once
#include <bitset>

#include "types/inttypes.hpp"
#include "window/StrobeKeys.hpp"

namespace sge {

class Keyboard {
 protected:
  std::bitset<STROBE_KEY_LAST + 1> keys;

 public:
  inline boolean isKeyDown(key_t key) const { return keys[key]; }
};

}  // namespace sge
