#pragma once
#include <memory>
#include <vector>

#include "renderer/vulkan/VBindable.hpp"

namespace sge::vulkan {

class VRenderable {
 public:
  VRenderable(const std::string& vertexShaderPath,
              const std::string& fragmentShaderPath,
              const std::vector<std::shared_ptr<VBindable>>& bindables,
              bool drawIndexed, u32 drawCount, u32 instanceCount)
      : m_vertexShaderPath(vertexShaderPath),
        m_fragmentShaderPath(fragmentShaderPath),
        m_bindables(bindables),
        m_drawIndexed(drawIndexed),
        m_drawCount(drawCount),
        m_instanceCount(instanceCount) {}

  std::string m_vertexShaderPath;
  std::string m_fragmentShaderPath;
  std::vector<std::shared_ptr<VBindable>> m_bindables;
  bool m_drawIndexed;
  u32 m_drawCount;
  u32 m_instanceCount;
};

}  // namespace sge::vulkan
