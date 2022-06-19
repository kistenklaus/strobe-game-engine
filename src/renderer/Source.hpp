#pragma once

#include <cassert>

#include "renderer/RenderPass.hpp"

namespace sge {

class RenderPass;

class ISource {
 public:
  RenderPass& getRenderPass();

 protected:
  ISource(RenderPass& renderPass);

 protected:
  RenderPass& m_renderPass;
  void* m_resource;
};

template <class Resource_t>
class Source : public ISource {
 public:
  Source(RenderPass& renderPass) : ISource(renderPass) {}
  Resource_t& get() {
    assert(m_resource != nullptr);
    return *reinterpret_cast<Resource_t*>(m_resource);
  }
  void set(Resource_t& resource) { m_resource = &resource; }
};

};  // namespace sge
