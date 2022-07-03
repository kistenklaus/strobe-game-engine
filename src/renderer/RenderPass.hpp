#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "renderer/Renderer.hpp"
#include "renderer/Sink.hpp"
#include "renderer/Source.hpp"
#include "types/inttypes.hpp"

namespace sge {

#define m_vrenderer (reinterpret_cast<VulkanRendererBackend*>(m_renderer))

class ISink;
template <class Resource_t>
class Sink;

class RenderPass {
 public:
  RenderPass(RendererBackend* renderer, const std::string name);
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

  const std::string getName() const { return m_name; }

  u32 getSinkIdByName(const std::string name) const;
  u32 getSourceIdByName(const std::string name) const;

 protected:
  template <class Resource_t>
  u32 registerSource(const std::string name) {
    u32 id = m_sources.size();
    std::unique_ptr<ISource> source =
        std::unique_ptr<Source<Resource_t>>(new Source<Resource_t>(*this));
    m_sources.push_back(std::move(source));
    m_sourceNames.push_back(name);
    return id;
  }

  template <class Resource_t>
  u32 registerSink(const std::string name) {
    u32 id = m_sinks.size();
    std::unique_ptr<ISink> sink =
        std::unique_ptr<Sink<Resource_t>>(new Sink<Resource_t>(*this));
    m_sinks.push_back(std::move(sink));
    m_sinkNames.push_back(name);
    return id;
  }

  template <class Resource_t>
  Resource_t* getSinkResource(u32 sink_id) {
    assert(sink_id < m_sinks.size());
    assert(m_sinks[sink_id]);  // raise if nullptr
    ISink* isink = m_sinks[sink_id].get();
    Sink<Resource_t>* sink = static_cast<Sink<Resource_t>*>(isink);
    return sink->get();
  }
  template <class Resource_t>
  void setSourceResource(u32 source_id, const Resource_t* p_resource) {
    assert(source_id < m_sources.size());
    assert(m_sources[source_id]);  // raise if nullptr
    ISource* p_isource = m_sources[source_id].get();
    Source<Resource_t>* p_source = static_cast<Source<Resource_t>*>(p_isource);
    p_source->set(p_resource);
  }

 private:
 protected:
  RendererBackend* m_renderer;

 private:
  const std::string m_name;
  std::vector<std::unique_ptr<ISink>> m_sinks;
  std::vector<std::string> m_sinkNames;
  std::vector<std::unique_ptr<ISource>> m_sources;
  std::vector<std::string> m_sourceNames;
};

}  // namespace sge
