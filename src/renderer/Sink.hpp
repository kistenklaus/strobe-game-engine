#pragma once
#include "renderer/RenderPass.hpp"
#include "renderer/Source.hpp"

namespace sge {

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

ISink::ISink(RenderPass& pass) : m_pass(pass) {}
RenderPass& ISink::getPass() { return m_pass; }
void ISink::link(ISource& source) { m_source = &source; }

template <class Resource_t>
class Sink : public ISink {
 public:
  Sink(RenderPass& renderPass);
  Resource_t& get();
};

template <class Resource_t>
Sink<Resource_t>::Sink(RenderPass& renderPass) : ISink(renderPass) {}

template <class Resource_t>
Resource_t& Sink<Resource_t>::get() {
  return static_cast<Source<Resource_t>>(m_source).get();
}

}  // namespace sge
