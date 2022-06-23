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
  void* mp_resource = nullptr;
};

template <class Resource_t>
class Source : public ISource {
 public:
  Source(RenderPass& renderPass) : ISource(renderPass) {}
  Resource_t* get() {
    assert(mp_resource != nullptr);
    return static_cast<Resource_t*>(mp_resource);
  }
  void set(const Resource_t* p_resource) {
    mp_resource = const_cast<Resource_t*>(p_resource);
  }
};

};  // namespace sge
