
#include "renderer/Renderer.hpp"
#include "window/Window.hpp"
using namespace sge;

int main() {
  Window window =
      Window(800, 600, "FLOATING", VULKAN_RENDER_BACKEND, GLFW_WINDOW_BACKEND);

  Renderer renderer = Renderer(VULKAN_RENDER_BACKEND, window);

  while (!window.shouldClose()) {
    window.pollEvents();
    renderer.beginFrame();

    renderer.renderFrame();
    renderer.endFrame();
    window.swapBuffers();
  }
}
