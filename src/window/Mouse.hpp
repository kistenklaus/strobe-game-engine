#pragma once
#include <bitset>

#include "types/inttypes.hpp"
#include "window/StrobeKeys.hpp"

namespace sge {

class Mouse {
 protected:
  double x;
  double y;
  double px;
  double py;
  double ds;
  std::bitset<STROBE_MOUSE_BUTTON_LAST + 1> buttons;

 public:
  virtual ~Mouse() = default;
  inline double getX() const { return x; }
  inline double getY() const { return y; }
  inline double getDX() const { return x - px; }
  inline double getDY() const { return y - py; }
  inline double getScroll() const { return ds; }
  inline boolean isButtonDown(button_t button) const { return buttons[button]; }
};

}  // namespace sge
