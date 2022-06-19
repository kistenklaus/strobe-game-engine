#pragma once
#include <memory>

namespace sge {

// Renderer Subclass interface class
class RendererBackend {
 public:
  virtual ~RendererBackend() = default;
  virtual void beginFrame() = 0;
  virtual void renderFrame() = 0;
  virtual void endFrame() = 0;

 private:
};

}  // namespace sge
