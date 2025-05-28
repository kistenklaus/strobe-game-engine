#pragma once

#include <memory>

#include "renderer/RenderContext.hpp"
#include "window/Window.hpp"
namespace strobe::renderer::details {

struct RendererControlBlock {
  strobe::window::Window window;
  std::shared_ptr<RenderContext> context;
};

}  // namespace strobe::renderer
