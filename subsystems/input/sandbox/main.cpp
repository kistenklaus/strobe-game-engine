#include <fmt/printf.h>
#include <strobe/lina.hpp>
#include <strobe/window/WindowManager.hpp>
#include <thread>

#include "strobe/input/InputSystem.hpp"

using namespace strobe;

int main() {
  WindowManager windowManager{};
  auto window = windowManager.createWindow(uvec2(800, 600), "abc");

  InputSystem inputSys{&windowManager};

  Keyboard keyboard = inputSys.createKeyboard();
  keyboard.addSource(window);

  inputSys.pollEvents();

  while (!window->closed()) {
    inputSys.pollEvents();
    if (keyboard.isKeyDown(Key::Space)) {
      fmt::println("DOWN");
    } else {
      fmt::println("UP");
    }
    std::this_thread::sleep_for(std::chrono::duration<float>(0.01));
  }
}
