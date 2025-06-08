#pragma once

#include <cstdint>
namespace strobe::window {

enum class Action : std::uint8_t { Release = 0, Press = 1, Repeat = 2 };

}
