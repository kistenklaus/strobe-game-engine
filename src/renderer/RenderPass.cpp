#include "renderer/RenderPass.hpp"

#include "renderer/SinkNotFoundException.hpp"
#include "renderer/SourceNotFoundException.hpp"

namespace sge {

RenderPass::RenderPass(RendererBackend* renderer, const std::string name,
                       const boolean executable)
    : m_renderer(renderer), m_name(name), m_exectuable(executable) {}

void RenderPass::linkSink(RenderPass& sourceRenderPass,
                          const std::string& sourceName,
                          const std::string& sinkName) {
  isource* p_source = nullptr;
  for (isource* p_isource : sourceRenderPass.m_sources) {
    if (p_isource->m_name == sourceName) {
      p_source = p_isource;
      break;
    }
  }
  if (p_source == nullptr) {
    throw SourceNotFoundException();
  }
  isink* p_sink = nullptr;
  for (isink* p_isink : m_sinks) {
    if (p_isink->m_name == sinkName) {
      p_sink = p_isink;
    }
  }
  if (p_sink == nullptr) {
    throw SinkNotFoundException();
  }
  p_sink->link(p_source);
}

}  // namespace sge
