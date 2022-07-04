
#include <cassert>
#include <chrono>

#include "logging/log.hpp"
#include "renderer/Renderer.hpp"
#include "window/Window.hpp"
using namespace sge;

int main() {
  println();
  Window window =
      Window(800, 600, "FLOATING", VULKAN_RENDER_BACKEND, GLFW_WINDOW_BACKEND);

  Renderer renderer = Renderer(VULKAN_RENDER_BACKEND, &window);

  while (!window.shouldClose()) {
    auto start = std::chrono::high_resolution_clock::now();
    window.pollEvents();
    renderer.beginFrame();
    renderer.renderFrame();
    renderer.endFrame();
    window.swapBuffers();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    println(duration.count() / 1e9);
  }
}
