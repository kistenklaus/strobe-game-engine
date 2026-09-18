#pragma once

#include "strobe/core/lina/vec.hpp"
namespace strobe::platform {

struct WindowInfo {
  uvec2 size;
  const char *title;
  bool focus = false;
  bool focusOnShow = false;
  bool resizable = false;
  bool decorated = false;
  bool floating = false;
};

} // namespace strobe::platform
