#pragma once
#include <memory>
#include <string>
#include <vector>

#include "renderer/Bindable.hpp"
#include "renderer/handles.hpp"

namespace sge::vulkan {

class VDrawJob {
 public:
  explicit VDrawJob(const std::string vertexShaderPath,
                    const std::string fragmentShaderPath,
                    const std::vector<std::shared_ptr<Bindable>> bindables,
                    const u32 pipelineIndex)
      : m_vertexShaderPath(vertexShaderPath),
        m_fragmentShaderPath(fragmentShaderPath),
        m_bindables(bindables),
        m_pipelineIndex(pipelineIndex) {}
  const std::string m_vertexShaderPath;
  const std::string m_fragmentShaderPath;
  const std::vector<std::shared_ptr<Bindable>> m_bindables;
  const u32 m_pipelineIndex;
};

}  // namespace sge::vulkan
