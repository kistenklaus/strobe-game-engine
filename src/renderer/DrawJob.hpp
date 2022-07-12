#pragma once
#include "renderer/RenderQueuePass.hpp"

namespace sge {

// fwd declaration
class RenderQueuePass;

class DrawJob {
  friend RenderQueuePass;

 public:
 private:
  RenderQueuePass* queue;
};

}  // namespace sge
