#pragma once
#include <memory>

namespace sge {

class RendererBackend {
 public:
  virtual ~RendererBackend() = default;
  virtual void beginFrame() = 0;
  virtual void renderFrame() = 0;
  virtual void endFrame() = 0;

 private:
 protected:
};

}  // namespace sge
