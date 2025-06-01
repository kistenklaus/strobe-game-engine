#include <print>
#include <strobe/window/WindowManager.hpp>
#include <strobe/lina.hpp>
#include <thread>
#include <chrono>

#include "strobe/input/InputSystem.hpp"

using namespace strobe;
using namespace std::chrono_literals;

int main() {
  WindowManager windowManager{};
  auto window = windowManager.createWindow(uvec2(800, 600), "abc");

  InputSystem inputSys{&windowManager};

  Keyboard keyboard = inputSys.createKeyboard();
  keyboard.addSource(window);
  
}
