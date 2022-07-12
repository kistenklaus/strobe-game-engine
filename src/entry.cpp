
#include <cassert>
#include <chrono>
#include <regex>

#include "logging/log.hpp"
#include "profiling/Profiler.hpp"
#include "renderer/Renderer.hpp"
#include "types/sarray.hpp"
#include "window/Window.hpp"

using namespace sge;

int main() {
  println();
  Window window =
      Window(800, 600, "FLOATING", VULKAN_RENDER_BACKEND, GLFW_WINDOW_BACKEND);

  Renderer renderer = Renderer(VULKAN_RENDER_BACKEND, &window);

  while (!window.shouldClose()) {
    Profiler::beginFrame();
    window.pollEvents();
    renderer.beginFrame();
    renderer.renderFrame();
    renderer.endFrame();
    window.swapBuffers();
    Profiler::endFrame();
    Profiler::logInfo();
  }
}
