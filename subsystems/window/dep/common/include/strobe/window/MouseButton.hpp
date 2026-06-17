#pragma once

#include <cstdint>
namespace strobe::window {

enum class MouseButton : uint8_t {
  Left = 0,
  Right = 1,
  Middle = 2,
  Extra1 = 3,
  Extra2 = 4,
  Extra3 = 5,
  Extra4 = 6,
  Extra5 = 7,
  Last = Extra5,
};
static constexpr std::size_t MOUSE_BUTTON_COUNT =
    static_cast<std::size_t>(MouseButton::Last) + 1;

}  // namespace strobe::window
