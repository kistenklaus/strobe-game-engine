#include "renderer/RenderPass.hpp"

namespace sge {

RenderPass::RenderPass(RendererBackend* renderer, const std::string name,
                       const boolean executable)
    : m_renderer(renderer), m_name(name), m_exectuable(executable) {}

void RenderPass::linkSink(u32 sink_id, RenderPass& source_pass, u32 source_id) {
  assert(sink_id < m_sinks.size());
  assert(m_sinks[sink_id]);  // raise if nullptr
  assert(source_id < source_pass.m_sources.size());
  assert(source_pass.m_sources[source_id]);  // raise if nullptr
  m_sinks[sink_id]->link(source_pass.m_sources[source_id].get());
}

u32 RenderPass::getSinkIdByName(const std::string name) const {
  for (u32 i = 0; i < m_sinkNames.size(); i++) {
    if (m_sinkNames[i] == name) {
      return i;
    }
  }
  throw std::runtime_error(std::string("RenderPass:[") + m_name +
                           std::string("] doesn't have a sink named : [") +
                           name + std::string("]"));
}

u32 RenderPass::getSourceIdByName(const std::string name) const {
  for (u32 i = 0; i < m_sourceNames.size(); i++) {
    if (m_sourceNames[i] == name) {
      return i;
    }
  }
  throw std::runtime_error(std::string("RenderPass:[") + m_name +
                           std::string("] doesn't have a source named : [") +
                           name + std::string("]"));
}

}  // namespace sge
