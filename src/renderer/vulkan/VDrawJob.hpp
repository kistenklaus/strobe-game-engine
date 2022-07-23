#pragma once
#include <memory>
#include <string>
#include <vector>

#include "renderer/handles.hpp"
#include "renderer/vulkan/VBindable.hpp"
#include "renderer/vulkan/VRenderable.hpp"

namespace sge::vulkan {
enum VDrawCallType { DRAW_CALL_DEFAULT = 0, DRAW_CALL_INDEXED = 1 };

class VDrawJob {
 public:
  explicit VDrawJob(const std::shared_ptr<VRenderable> renderable,
                    u32 pipelineIndex)
      : m_renderable(renderable), m_pipelineIndex(pipelineIndex) {}
  const std::shared_ptr<VRenderable> m_renderable;
  const u32 m_pipelineIndex;
};

}  // namespace sge::vulkan
