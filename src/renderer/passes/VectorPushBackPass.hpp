#pragma once
#include "renderer/RenderPass.hpp"

namespace sge {

template <typename T>

class VectorPushBackPass : public RenderPass {
 public:
  explicit VectorPushBackPass(RendererBackend* renderer, const std::string name)
      : RenderPass(renderer, name),
        m_vecSink(registerSink<std::vector<T>>("vector")),
        m_valueSink(registerSink<T>("value")),
        m_outSrc(registerSource<std::vector<T>>("vector")) {}

  void execute() override {
    std::vector<T>* p_vec = *getSinkResource<std::vector<T>>(m_vecSink);
    p_vec->push_back(getSinkResource<T>(m_valueSink));
    setSourceResource(m_outSrc, p_vec);
  }

  u32 getVectorSink() const { return m_vecSink; }
  u32 getValueSink() const { return m_valueSink; }
  u32 getVectorSource() const { return m_outSrc; }

 private:
  const u32 m_vecSink;
  const u32 m_valueSink;
  const u32 m_outSrc;
};

}  // namespace sge
