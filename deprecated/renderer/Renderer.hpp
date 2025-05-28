#pragma once

#include <memory>
#include <mutex>
#include <utility>

#include "renderer/RenderBackend.hpp"
#include "renderer/RenderContext.hpp"
#include "renderer/RendererControlBlock.hpp"
#include "renderer/resources.hpp"
#include "window/Window.hpp"

namespace strobe::renderer {

class Renderer {
 public:
  Renderer(strobe::window::Window window)
      : m_controlBlock(std::make_shared<details::RendererControlBlock>()) {
    m_controlBlock->window = window;
    m_controlBlock->context = nullptr;
  }

  void beginFrame() {
    m_controlBlock->context->beginFrame();
  }

  ResourceHandle createText(std::string_view text) {
    return m_controlBlock->context->createText(text);
  }

  void destroyText(ResourceHandle text) {
    m_controlBlock->context->destroyText(text);
  }

  void drawText(ResourceHandle text) {
    m_controlBlock->context->drawText(text);
  }


  void endFrame() {
    m_controlBlock->context->endFrame();
  }

  std::shared_ptr<RenderContext> createContext(RenderBackend backend);

  std::shared_ptr<RenderContext> getContext() const {
    assert(m_controlBlock->context != nullptr);
    return m_controlBlock->context;
  }

 private:
  std::shared_ptr<details::RendererControlBlock> m_controlBlock;
};

}  // namespace strobe::renderer
