#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

#include <format>
#include <latch>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <thread>

#include "core/ApplicationInfo.hpp"
#include "logging/Logger.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/resources.hpp"
#include "resource/ResourceManager.hpp"
#include "resource/file/Path.hpp"
#include "window/Window.hpp"
#include "window/WindowApi.hpp"

using namespace strobe;
using namespace std::chrono_literals;

int main() {
  auto logger = logging::Logger::create();

  spdlog::set_level(spdlog::level::level_enum::debug);

  auto appInfo = std::make_shared<ApplicationInfo>();
  appInfo->name = "Vks-Testing";
  appInfo->version = {0, 0, 1};
  appInfo->engine.name = "strobe";
  appInfo->engine.version = {0, 42, 0};

  auto resourceManager = resource::ResourceManager::create();
  {
    auto file = resourceManager.loadFile(
        resource::Path::toResource("shaders/test.vert.glsl"));
    if (!file) {
      throw std::runtime_error(file.error()->message());
    }
    std::println("{}", file->text());

    auto file2 = resourceManager.loadFile(
        resource::Path::toResource("shaders/test.vert.glsl"));
    if (!file2) {
      throw std::runtime_error(file2.error()->message());
    }
    std::println("{}", file2->text());
  }

  window::Window window{appInfo};
  renderer::Renderer renderer{window};

  renderer.createContext(renderer::RenderBackend::Vks)->start();

  auto eventloop = window.createContext(strobe::window::WindowApi::GLFW);
  // we would have to wait until the renderer is initalized.

  int x = 0;
  while (!eventloop->shouldClose()) {
    eventloop->pollEvents();

    renderer.beginFrame();
    renderer::ResourceHandle text =
        renderer.createText(std::format("Frame {}", x++));
    renderer.drawText(text);
    renderer.destroyText(text);

    renderer.endFrame();

    std::this_thread::sleep_for(100ms);
  }
  renderer.getContext()->stop().wait();

  resourceManager.destroy().wait();

  logger.destroy();
}
