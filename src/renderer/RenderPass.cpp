#include "renderer/RenderPass.hpp"

namespace sge {

RenderPass::RenderPass(RendererBackend* renderer) : m_renderer(renderer) {}

void RenderPass::linkSink(u32 sink_id, RenderPass& source_pass, u32 source_id) {
  assert(sink_id < m_sinks.size());
  assert(m_sinks[sink_id]);  // raise if nullptr
  assert(source_id < source_pass.m_sources.size());
  assert(source_pass.m_sources[source_id]);  // raise if nullptr
  m_sinks[sink_id]->link(source_pass.m_sources[source_id].get());
}

}  // namespace sge
