#pragma once
#include <cassert>
#include <typeindex>

#include "renderer/RenderPass.hpp"
#include "renderer/SinkInvalidValueException.hpp"
#include "renderer/SinkNotLinkedException.hpp"
#include "renderer/SinkNullPointerException.hpp"
#include "renderer/Source.hpp"

namespace sge {

class RenderPass;
class ISource;
template <class Resource_t>
class Source;

class ISink {
 public:
  RenderPass& getPass();
  void link(ISource* p_source);

  virtual const std::type_index getResourceType() const = 0;

 protected:
  ISink(RenderPass& pass, const boolean nullable);

 protected:
  ISource* mp_source = nullptr;
  const boolean m_nullable;

 private:
  RenderPass& m_pass;
};

template <class Resource_t>
class Sink : public ISink {
 public:
  Sink(RenderPass& renderPass, const boolean nullable)
      : ISink(renderPass, nullable) {}
  Resource_t* get() {
    if (mp_source == nullptr) {
      throw SinkNotLinkedException();
    }
    Resource_t* p_res = static_cast<Source<Resource_t>*>(mp_source)->get();
    if (!m_nullable && p_res == nullptr) {
      throw SinkNullPointerException();
    }
    return p_res;
  }
  const std::type_index getResourceType() const override {
    //
    return std::type_index(typeid(Resource_t));
  }
};

}  // namespace sge
