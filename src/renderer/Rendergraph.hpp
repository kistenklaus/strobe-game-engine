#pragma once

#include <set>
#include <vector>

#include "renderer/RenderPass.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Resource.hpp"
#include "renderer/Sink.hpp"
#include "renderer/Source.hpp"
#include "types/inttypes.hpp"

namespace sge {

class Rendergraph : public RenderPass {
 public:
  Rendergraph(Renderer& renderer);

  virtual void beginFrame() override;
  virtual void execute() override;
  virtual void endFrame() override;
  virtual void dispose() override;

 protected:
  void addLinkage(u32 source_pass_id, u32 source_id, u32 sink_pass_id,
                  u32 sink_id);
  template <class RenderPass_t, typename... Args>
  u32 createPass(Args&&... args);
  template <class RenderPass_t>
  RenderPass_t& getPassById(u32 pass_id);
  RenderPass& getPassById(u32 pass_id);
  void build();

 private:
 private:
  std::vector<std::pair<std::pair<u32, u32>, std::pair<u32, u32>>> m_linkages;
  std::vector<u32> m_execution_order;
  std::vector<std::unique_ptr<RenderPass>> m_passes;
  std::vector<std::unique_ptr<ISource>> m_global_resource_sources;
  boolean m_is_deprecated = false;
};

Rendergraph::Rendergraph(Renderer& renderer) : RenderPass(renderer) {}

void Rendergraph::beginFrame() {
  if (m_is_deprecated) {
    build();
  }
  for (const u32 pass_id : m_execution_order) {
    getPassById(pass_id).beginFrame();
  }
}

void Rendergraph::execute() {
  for (const u32 pass_id : m_execution_order) {
    getPassById(pass_id).execute();
  }
}

void Rendergraph::endFrame() {
  for (const u32 pass_id : m_execution_order) {
    getPassById(pass_id).endFrame();
  }
}

void Rendergraph::dispose() {
  for (const u32 pass_id : m_execution_order) {
    getPassById(pass_id).dispose();
  }
}

void Rendergraph::addLinkage(u32 source_pass_id, u32 source_id,
                             u32 sink_pass_id, u32 sink_id) {
  assert(source_pass_id <= m_passes.size());
  assert(sink_pass_id <= m_passes.size());
  m_linkages.push_back(std::make_pair(std::make_pair(source_pass_id, source_id),
                                      std::make_pair(sink_pass_id, sink_id)));
}

template <class RenderPass_t, typename... Args>
u32 Rendergraph::createPass(Args&&... args) {
  // i have no idea if i need std::forward here.
  m_passes.push_back(
      std::make_unique<RenderPass_t>(std::forward<Args>(args)...));
  // intentionally returns the vector index +1.
  return m_passes.size();
}

void Rendergraph::build() {
  // 1. link sinks to sources.
  for (const auto& linkage : m_linkages) {
    const u32 source_pass_id = linkage.first.first;
    const u32 source_id = linkage.first.second;
    const u32 sink_pass_id = linkage.second.first;
    const u32 sink_id = linkage.second.second;
    m_passes[sink_pass_id - 1]->linkSink(sink_id, getPassById(source_pass_id),
                                         source_id);
    m_execution_order.clear();
  }
  std::vector<u32> sort;

  std::vector<std::vector<u32>> graph(m_passes.size());
  for (const auto& linkage : m_linkages) {
    const u32 source_pass_id = linkage.first.first;
    // const u32 source_id = linkage.first.second;
    const u32 sink_pass_id = linkage.second.first;
    // const u32 sink_id = linkage.second.second;
    graph[source_pass_id].push_back(sink_pass_id);
  }
  // TODO Topological sort
  m_is_deprecated = false;
}

template <class RenderPass_t>
RenderPass_t& Rendergraph::getPassById(u32 pass_id) {
  assert(pass_id <= m_passes.size());
  return *reinterpret_cast<const RenderPass_t*>(m_passes[pass_id - 1].get());
}

RenderPass& Rendergraph::getPassById(u32 pass_id) {
  assert(pass_id <= m_passes.size());
  return *m_passes[pass_id - 1];
}

}  // namespace sge
