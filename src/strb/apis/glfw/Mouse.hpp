#pragma once
#include "strb/apis/glfw/glfw.lib.hpp"
#include "strb/core/window/Mouse.hpp"
#include "strb/core/window/strobe_keys.hpp"

namespace strb::glfw {

class Mouse : public strb::window::Mouse {

public:
  ~Mouse() override {}
  inline void setPos(double xpos, double ypos) {
    this->x = xpos;
    this->y = ypos;
  }
  inline void setPPos(double pxpos, double pypos) {
    this->px = pxpos;
    this->py = pypos;
  }
  inline void rotate() {
    this->px = this->x;
    this->py = this->y;
  }
  inline void setDScroll(double ds) { this->ds = ds; }
  inline void setKeyState(button_t button, action_t action) {
    buttons[button] = (action == GLFW_PRESS);
  }
};

} // namespace strb::glfw
