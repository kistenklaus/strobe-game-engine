#pragma once

#include <cassert>

#include "renderer/RenderPass.hpp"

namespace sge {

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
  Source(RenderPass& renderPass);
  Resource_t& get();
  void set(Resource_t& resource);
};

// Implementation::
ISource::ISource(RenderPass& renderPass) : m_renderPass(renderPass) {}
RenderPass& ISource::getRenderPass() { return m_renderPass; }

template <class Resource_t>
Source<Resource_t>::Source(RenderPass& renderPass) : ISource(renderPass) {}

template <class Resource_t>
Resource_t& Source<Resource_t>::get() {
  assert(m_resource != nullptr);
  return *reinterpret_cast<Resource_t*>(m_resource);
}

template <class Resource_t>
void Source<Resource_t>::set(Resource_t& resource) {
  m_resource = &resource;
}

};  // namespace sge
