#pragma once
#include "renderer/RenderQueuePass.hpp"
#include "renderer/handles.hpp"

namespace sge {

// fwd declaration
class RenderQueuePass;

class DrawJob {
  friend RenderQueuePass;

 public:
 private:
  RenderQueuePass* queue;
  shader_module vertexShader;
};

}  // namespace sge
