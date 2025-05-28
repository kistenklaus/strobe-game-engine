#include <GLFW/glfw3.h>

#include <chrono>
#include <functional>
#include <print>
#include <strobe/memory.hpp>
#include <strobe/window/WindowSubsystem.hpp>
#include <thread>
#include "strobe/window/WindowImpl.hpp"

int main() {
  using namespace strobe;
  using namespace std::chrono_literals;

  WindowSubsystem<Mallocator> window{
      uvec2(800, 600),
      "my-cool-title",
  };
  window.setResizable(true);

  std::println("Window Created");

  std::function<void(const window::KeyboardEvent&)> callback =
      [](const window::KeyboardEvent& event) {
        std::println("Keyboard event");
        return;
      };
  auto handle = window.addKeyboardListener(
      EventListenerRef<window::KeyboardEvent>::fromCallable(&callback));

  std::function<void(const window::ResizeEvent&)> sizeCallback =
      [](const window::ResizeEvent& event) {
        std::println("Resize event");
        return;
      };

  auto handle2 = window.addFramebufferSizeListener(
      EventListenerRef<window::ResizeEvent>::fromCallable(&sizeCallback));

  std::function<void(const window::MouseButtonEvent&)> mouseButtonCallback =
      [](const window::MouseButtonEvent& event) {
        std::println("Mouse event");
        return;
      };

  auto handle3 = window.addMouseButtonEventListener(
      EventListenerRef<window::MouseButtonEvent>::fromCallable(
          &mouseButtonCallback));

  while (!window.closed()) {
    std::this_thread::yield();
  }
  return 0;
}
