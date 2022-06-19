#pragma once
#include "window/Keyboard.hpp"
#include "window/glfw/glfw.lib.hpp"

namespace sge::glfw {

class GlfwKeyboard : public Keyboard {
 public:
  inline void setKeyState(key_t key, action_t action) {
    keys[key] = action == GLFW_PRESS;
  }
};

}  // namespace sge::glfw
