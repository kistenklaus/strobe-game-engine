#include "renderer/Rendergraph.hpp"

#include <iostream>

#include "algorithms/graph.hpp"
#include "logging/print.hpp"

namespace sge {

Rendergraph::Rendergraph(RendererBackend* renderer) : RenderPass(renderer) {}

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

void Rendergraph::markPassAsRoot(const u32 passId) {
  m_rootPassIds.push_back(passId);
}

void Rendergraph::unmarkPassAsRoot(const u32 passId) {
  for (u32 i = 0; i < m_rootPassIds.size(); i++) {
    if (m_rootPassIds[i] == passId) {
      m_rootPassIds.erase(m_rootPassIds.begin() + i);
      return;
    }
  }
}

void Rendergraph::build() {
  // 1. link sinks to sources.
  for (const auto& linkage : m_linkages) {
    const u32 source_pass_id = linkage.first.first;
    const u32 source_id = linkage.first.second;
    const u32 sink_pass_id = linkage.second.first;
    const u32 sink_id = linkage.second.second;
    m_passes[sink_pass_id]->linkSink(sink_id, getPassById(source_pass_id),
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
    graph[sink_pass_id].push_back(source_pass_id);
  }
  // TODO Topological sort
  m_execution_order = selectiveReverseTopoligicalSort(graph, m_rootPassIds);
  print("execution order:");
  println(m_execution_order);
  m_is_deprecated = false;
}

RenderPass& Rendergraph::getPassById(u32 pass_id) {
  assert(pass_id <= m_passes.size());
  return *m_passes[pass_id];
}
}  // namespace sge
