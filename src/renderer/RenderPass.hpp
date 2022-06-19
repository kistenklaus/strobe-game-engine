#pragma once
#include <cassert>
#include <memory>
#include <vector>

#include "renderer/Renderer.hpp"
#include "renderer/Sink.hpp"
#include "renderer/Source.hpp"
#include "types/inttypes.hpp"

namespace sge {

class RenderPass {
 public:
  RenderPass(Renderer& renderer);
  virtual ~RenderPass() = default;
  RenderPass(RenderPass&) = delete;
  RenderPass(RenderPass&&) = default;
  RenderPass& operator=(RenderPass&) = delete;
  virtual void recreate() {}
  virtual void beginFrame() {}
  virtual void execute() {}
  virtual void endFrame() {}
  virtual void dispose() {}

  void linkSink(u32 sink_id, RenderPass& source_pass, u32 source_id);

 protected:
  template <class Resource_t>
  u32 registerSource();
  template <class Resource_t>
  u32 registerSink();
  template <class Resource_t>
  Resource_t& getSinkResource(u32 sink_id);
  template <class Resource_t>
  void setSourceResource(u32 source_id, Resource_t& resource);

 protected:
  const Renderer& m_renderer;
  std::vector<std::unique_ptr<ISink>> m_sinks;
  std::vector<std::unique_ptr<ISource>> m_sources;
};

RenderPass::RenderPass(Renderer& renderer) : m_renderer(renderer) {}

template <class Resource_t>
u32 RenderPass::registerSource() {
  u32 id = m_sources.size();
  m_sources.push_back(std::make_unique<Source<Resource_t>>(*this));
  return id;
}

template <class Resource_t>
u32 RenderPass::registerSink() {
  u32 id = m_sinks.size();
  m_sinks.push_back(std::make_unique<Sink<Resource_t>>(*this));
  return id;
}

template <class Resource_t>
Resource_t& RenderPass::getSinkResource(u32 sink_id) {
  assert(sink_id < m_sinks.size());
  assert(typeid(m_sinks[sink_id]) == typeid(Sink<Resource_t>));
  assert(m_sinks[sink_id]);  // raise if nullptr
  return reinterpret_cast<Sink<Resource_t>>(m_sinks[sink_id]).get();
}

template <class Resource_t>
void RenderPass::setSourceResource(u32 source_id, Resource_t& resource) {
  assert(source_id < m_sources.size());
  assert(typeid(m_sources[source_id]) == typeid(Source<Resource_t>));
  assert(m_sources[source_id]);  // raise if nullptr
  static_cast<Source<Resource_t>>(*m_sources[source_id]).set(resource);
}

void RenderPass::linkSink(u32 sink_id, RenderPass& source_pass, u32 source_id) {
  assert(sink_id < m_sinks.size());
  assert(m_sinks[sink_id]);  // raise if nullptr
  assert(source_id < source_pass.m_sources.size());
  assert(source_pass.m_sources[source_id]);  // raise if nullptr
  m_sinks[sink_id]->link(*source_pass.m_sources[source_id]);
}

}  // namespace sge

/*

constructor(){
  auto in_target = registerSink<Framebuffer>();
  auto out_target = registerSource<Framebuffer>();
}

renderPass.execute(){

  //do shit with in_target;
  Framebuffer& getSinkResource<Framebuffer>(in_target);

  setSourceResource(out_target, target);
}

*/
