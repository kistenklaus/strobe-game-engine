#include "renderer/Rendergraph.hpp"

#include <chrono>
#include <iostream>

#include "algorithms/graph.hpp"
#include "logging/log.hpp"
#include "logging/print.hpp"
#include "renderer/IncompatibleLinkageException.hpp"
#include "renderer/RendergraphResourcesDeprecatedException.hpp"

namespace sge {

Rendergraph::Rendergraph(RendererBackend* renderer, const std::string name)
    : RenderPass(renderer, name) {}

void Rendergraph::create() {
  if (m_is_deprecated) {
    build();
  }
  for (const u32 pass_id : m_dependencyOrder) {
    getPassById(pass_id).create();
  }
}

void Rendergraph::recreate() {
  if (m_is_deprecated) {
    build();
  }

  for (const u32 pass_id : m_dependencyOrder) {
    getPassById(pass_id).recreate();
  }
  m_requireRecreation = false;
}

void Rendergraph::beginFrame() {
  if (m_is_deprecated) {
    build();
  }
  if (m_requireRecreation) {
    recreate();
  }
  for (const u32 pass_id : m_execution_order) {
    getPassById(pass_id).beginFrame();
  }
}

void Rendergraph::execute() {
  try {
    for (const u32 pass_id : m_execution_order) {
      getPassById(pass_id).execute();
    }
  } catch (const RendergraphResourcesDeprecatedException& e) {
    recreate();
  }
}

void Rendergraph::endFrame() {
  for (const u32 pass_id : m_execution_order) {
    getPassById(pass_id).endFrame();
  }
}

void Rendergraph::dispose() {
  std::vector<u32> rev = m_dependencyOrder;
  rev.reserve(rev.size());
  for (const u32 pass_id : rev) {
    getPassById(pass_id).dispose();
  }
}

void Rendergraph::addLinkage(u32 source_pass_id, u32 source_id,
                             u32 sink_pass_id, u32 sink_id) {
  assert(source_pass_id <= m_passes.size());
  assert(sink_pass_id <= m_passes.size());
  // check for types!
  if (getPassById(source_pass_id).getSourceById(source_id)->getResourceType() !=
      getPassById(sink_pass_id).getSinkById(sink_id)->getResourceType()) {
    throw IncompatibleLinkageException(
        "typeof [" + getPassById(source_pass_id).getName() + "@" +
        getPassById(source_pass_id).getSourceNameById(source_id) +
        "] doesn't match the typeof [" + getPassById(sink_pass_id).getName() +
        "@" + getPassById(sink_pass_id).getSinkNameById(sink_id) + "]");
  }

  m_linkages.push_back(std::make_pair(std::make_pair(source_pass_id, source_id),
                                      std::make_pair(sink_pass_id, sink_id)));
}

void Rendergraph::addLinkage(const std::string source, const std::string sink) {
  u32 sindex = source.find('@');
  const std::string sourcePassName = source.substr(0, sindex);
  const std::string sourceName = source.substr(sindex + 1);
  sindex = sink.find('@');
  const std::string sinkPassName = sink.substr(0, sindex);
  const std::string sinkName = sink.substr(sindex + 1);
  const u32 sourcePassId = getPassIdByName(sourcePassName);
  const u32 sinkPassId = getPassIdByName(sinkPassName);
  const RenderPass& sourcePass = getPassById(sourcePassId);
  const u32 sourceId = sourcePass.getSourceIdByName(sourceName);
  const RenderPass& sinkPass = getPassById(sinkPassId);
  const u32 sinkId = sinkPass.getSinkIdByName(sinkName);
  addLinkage(sourcePassId, sourceId, sinkPassId, sinkId);
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
  }
  std::vector<u32> sort;

  std::vector<std::vector<u32>> graph(m_passes.size());
  for (const auto& linkage : m_linkages) {
    const u32 source_pass_id = linkage.first.first;
    const u32 sink_pass_id = linkage.second.first;
    graph[sink_pass_id].push_back(source_pass_id);
  }
  // TODO Topological sort
  m_dependencyOrder.clear();
  m_dependencyOrder = selectiveReverseTopoligicalSort(graph, m_rootPassIds);
  m_execution_order.clear();
  for (const u32 pass : m_dependencyOrder) {
    if (getPassById(pass).isExecutable()) {
      m_execution_order.push_back(pass);
    }
  }
  static const LogChannel channel =
      createLogChannel("rendergraph-execution-order", true);
  if (channel.isActive()) {
    std::vector<std::string> exeuction_names(m_dependencyOrder.size());

    for (u32 i = 0; i < exeuction_names.size(); i++) {
      exeuction_names[i] =
          std::string("\n\t") +
          (getPassById(m_dependencyOrder[i]).isExecutable() ? "" : "(") +
          getPassById(m_dependencyOrder[i]).getName() + std::string("[") +
          std::to_string(m_dependencyOrder[i]) + std::string("]") +
          (getPassById(m_dependencyOrder[i]).isExecutable() ? "" : ")") +
          (i == m_dependencyOrder.size() - 1 ? "\n" : "");
    }
    log(exeuction_names, channel);
  }

  m_is_deprecated = false;
}

RenderPass& Rendergraph::getPassById(u32 pass_id) {
  assert(pass_id <= m_passes.size());
  return *m_passes[pass_id];
}

RenderPass& Rendergraph::getPassByName(const std::string name) {
  for (const std::unique_ptr<RenderPass>& pass : m_passes) {
    if (pass->getName() == name) {
      return *pass;
    }
  }
  throw std::runtime_error(std::string("Rendergraph:[") + getName() +
                           std::string("] doesn't have a pass named: [") +
                           name + std::string("]"));
}

u32 Rendergraph::getPassIdByName(const std::string name) {
  for (u32 i = 0; i < m_passes.size(); i++) {
    if (m_passes[i]->getName() == name) {
      return i;
    }
  }
  throw std::runtime_error(std::string("Rendergraph:[") + getName() +
                           std::string("] doesn't have a pass named: [") +
                           name + std::string("]"));
}

}  // namespace sge
