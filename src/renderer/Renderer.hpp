
#pragma once

#include <memory>

#include "renderer/RendererBackend.hpp"
#include "renderer/RendererBackendAPI.hpp"
#include "window/Window.hpp"

namespace sge {

class Renderer {
 public:
  Renderer(RendererBackendAPI backendApi, Window* window);
  void beginFrame();
  void renderFrame();
  void endFrame();

 private:
  const std::unique_ptr<RendererBackend> m_backend;
};

}  // namespace sge
