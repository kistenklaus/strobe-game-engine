#include <GLFW/glfw3.h>

#include <chrono>
#include <print>
#include <strobe/memory.hpp>
#include <strobe/window/WindowSubsystem.hpp>
#include <thread>

#include "strobe/core/events/event_listener_handle.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/window/WindowImpl.hpp"

using namespace strobe;
using namespace std::chrono_literals;

class Keyboard {
 public:
  Keyboard(strobe::WindowSubsystem<strobe::Mallocator>& window) {
    m_listenerHandle = window.addKeyboardListener(
        EventListenerRef<window::KeyboardEvent>::fromMemberFunction<
            Keyboard, &Keyboard::callback>(this));

  }

  ~Keyboard() { std::println("Keyboard destructor"); }

  void callback(const strobe::window::KeyboardEvent& event) {
    std::println("EVETN");
    m_listenerHandle.release();
  }

  strobe::EventListenerHandle m_listenerHandle;
};

int main() {
  WindowSubsystem<Mallocator> window{
      uvec2(800, 600),
      "my-cool-title",
  };
  window.setResizable(true);

  Keyboard keyboard(window);
  // keyboard.m_listenerHandle.release();

  while (!window.closed()) {
    std::this_thread::sleep_for(100ms);
  }
  std::println("EXIT loop");
}
