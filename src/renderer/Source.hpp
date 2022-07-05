#pragma once

#include <cassert>
#include <typeindex>

#include "renderer/RenderPass.hpp"

namespace sge {

static u32 source_type_index_acc = 0;

class RenderPass;

class ISource {
 public:
  RenderPass& getRenderPass();
  virtual const std::type_index getResourceType() const = 0;

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
  Resource_t* get() { return static_cast<Resource_t*>(mp_resource); }
  void set(const Resource_t* p_resource) {
    mp_resource = const_cast<Resource_t*>(p_resource);
  }
  const std::type_index getResourceType() const override {
    return std::type_index(typeid(Resource_t));
  };
};

};  // namespace sge
