#include <GLFW/glfw3.h>
#include <unistd.h>

#include <format>
#include <latch>
#include <ostream>
#include <print>
#include <thread>

#include "renderer/Renderer.hpp"
#include "renderer/resources.hpp"
#include "window/ClientApi.hpp"
#include "window/Window.hpp"
#include "window/WindowApi.hpp"

using namespace strobe;
using namespace std::chrono_literals;


int main() {
  window::Window window;
  renderer::Renderer renderer{window};

  std::println("Creating renderer context");
  renderer.createContext(renderer::RenderBackend::Vks)->start();

  std::println("Init window & eventloop");
  auto eventloop = window.createContext(strobe::window::WindowApi::GLFW);
  // we would have to wait until the renderer is initalized.

  std::println("Enter event loop");
  int x = 0;
  while (!eventloop->shouldClose()) {
    eventloop->pollEvents();

    renderer.beginFrame();
    renderer::ResourceHandle text = renderer.createText(std::format("Frame {}", x++));
    renderer.drawText(text);
    renderer.destroyText(text);

    renderer.endFrame();

    std::this_thread::sleep_for(100ms);

  }
  renderer.getContext()->stop().wait();
  std::println("Exit event thread");
}
