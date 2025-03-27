#pragma once
#include <cstdint>
#include "GLFW/glfw3.h"

namespace strobe::input {

enum class Action : std::uint8_t {
  Press,
  Release,
  Repeat
};

}
