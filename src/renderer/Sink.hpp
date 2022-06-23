#pragma once
#include <cassert>

#include "renderer/RenderPass.hpp"
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

 protected:
  ISink(RenderPass& pass);

 protected:
  ISource* mp_source = nullptr;

 private:
  RenderPass& m_pass;
};

template <class Resource_t>
class Sink : public ISink {
 public:
  Sink(RenderPass& renderPass) : ISink(renderPass) {}
  Resource_t* get() {
    assert(mp_source != nullptr);
    return static_cast<Source<Resource_t>*>(mp_source)->get();
  }
};

}  // namespace sge
