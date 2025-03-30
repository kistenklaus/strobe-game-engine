#include "./Renderer.hpp"
#include "renderer/vks/VksRenderer.hpp"

namespace strobe::renderer {

std::shared_ptr<RenderContext> Renderer::createContext(RenderBackend backend) {
  switch (backend) {
    case RenderBackend::Vks: {
      m_controlBlock->context =
          std::make_shared<vks::VksRenderer>(m_controlBlock->window);
      return m_controlBlock->context;
    }
    default:
      std::unreachable();
  }
}
}  // namespace strobe::renderer
