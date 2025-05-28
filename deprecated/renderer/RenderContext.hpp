#pragma once

#include <future>
#include "renderer/resources.hpp"
namespace strobe::renderer {

class RenderContext {
 public:
  virtual ~RenderContext() = default;

  virtual void beginFrame() = 0;
  virtual void endFrame() = 0;

  virtual ResourceHandle createText(std::string_view text) = 0;
  virtual void drawText(ResourceHandle text) = 0;
  virtual void destroyText(ResourceHandle text) = 0;

  virtual void start() = 0;
  virtual std::future<void> stop() = 0;

 protected:
};

}  // namespace strobe::renderer
