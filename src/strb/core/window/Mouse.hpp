#pragma once
#include "strb/core/window/strobe_keys.hpp"
#include "strb/stl.hpp"

namespace strb::window {

class Mouse {
protected:
  double x;
  double y;
  double px;
  double py;
  double ds;
  strb::bitset<STROBE_MOUSE_BUTTON_LAST + 1> buttons;

public:
  virtual ~Mouse() = default;
  inline double getX() const { return x; }
  inline double getY() const { return y; }
  inline double getDX() const { return x - px; }
  inline double getDY() const { return y - py; }
  inline double getScroll() const { return ds; }
  inline strb::boolean isButtonDown(button_t button) const {
    return buttons[button];
  }
};

} // namespace strb::window
