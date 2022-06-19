#pragma once
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
  void link(ISource& source);

 protected:
  ISink(RenderPass& pass);

 protected:
  ISource* m_source;

 private:
  RenderPass& m_pass;
};

template <class Resource_t>
class Sink : public ISink {
 public:
  Sink(RenderPass& renderPass) : ISink(renderPass) {}
  Resource_t& get() {
    return static_cast<Source<Resource_t>*>(m_source)->get();
  }
};

}  // namespace sge
