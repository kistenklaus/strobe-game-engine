#pragma once
#include "strb/core/window/Keyboard.hpp"
#include <GLFW/glfw3.h>

namespace strb::glfw {

class Keyboard : public strb::window::Keyboard {
public:
  inline void setKeyState(key_t key, action_t action) {
    keys[key] = action == GLFW_PRESS;
  }
};

} // namespace strb::glfw
