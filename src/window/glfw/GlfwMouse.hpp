#pragma once
#include "window/Mouse.hpp"
#include "window/StrobeKeys.hpp"
#include "window/glfw/glfw.lib.hpp"

namespace sge::glfw {

class GlfwMouse : public Mouse {
 public:
  ~GlfwMouse() override {}
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

}  // namespace sge::glfw
