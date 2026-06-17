#include <strobe/memory.hpp>
#include <strobe/window/WindowManager.hpp>
#include <strobe/lina.hpp>
#include <thread>

#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/window/KeyboardEvent.hpp"

using namespace strobe;
using namespace std::chrono_literals;

int main() {
  WindowManager subsys{};

  WindowHandle win = subsys.createWindow(uvec2(800, 600), "idk", false);
  auto handle = win->addKeyboardEventListener(
      EventListenerRef<window::KeyboardEvent>::fromNative(
          nullptr, [](void *, const window::KeyboardEvent &event) {

          }));

  while (!win->closed()) {
    std::this_thread::yield();
  }
}
