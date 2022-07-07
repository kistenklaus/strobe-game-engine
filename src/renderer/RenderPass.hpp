#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "renderer/Renderer.hpp"
#include "renderer/SinkNotLinkedException.hpp"
#include "renderer/SinkNullPointerException.hpp"
#include "renderer/sinksource.hpp"
#include "types/inttypes.hpp"

namespace sge {

#define m_vrenderer (reinterpret_cast<VRendererBackend*>(m_renderer))

class ISink;
template <class Resource_t>
class Sink;

class RenderPass {
 public:
  RenderPass(RendererBackend* renderer, const std::string name,
             const boolean executable = true);
  virtual ~RenderPass() = default;
  RenderPass(RenderPass&) = delete;
  RenderPass(RenderPass&&) = default;
  RenderPass& operator=(RenderPass&) = delete;
  virtual void create() {}
  virtual void recreate() {}
  virtual void beginFrame() {}
  virtual void execute() {}
  virtual void endFrame() {}
  virtual void dispose() {}

  void linkSink(RenderPass& sourceRenderPass, const std::string& sourceName,
                const std::string& sinkName);

  const std::string getName() const { return m_name; }
  const boolean isExecutable() const { return m_exectuable; }

 protected:
  void registerSource(isource* source) {
    // TODO move to cpp
    source->m_pass = this;
    m_sources.push_back(source);
  }
  void registerSink(isink* sink) {
    // TODO move to cpp
    sink->m_pass = this;
    m_sinks.push_back(sink);
  }
  void registerSinkSource(isinksource* sinksource) {
    registerSink(sinksource->getSink());
    registerSource(sinksource->getSource());
  }

 private:
 protected:
  RendererBackend* m_renderer;

 private:
  const std::string m_name;
  const boolean m_exectuable;
  std::vector<isink*> m_sinks;
  std::vector<isource*> m_sources;
};

}  // namespace sge
