#pragma once
#include "renderer/RenderPass.hpp"

namespace sge {

template <typename T>

class VectorPushBackPass : public RenderPass {
 public:
  explicit VectorPushBackPass(RendererBackend* renderer)
      : RenderPass(renderer),
        m_vecSink(registerSink<std::vector<T>>()),
        m_valueSink(registerSink<T>()),
        m_outSrc(registerSource<std::vector<T>>()) {}

  void execute() override {
    std::vector<T>* p_vec = *getSinkResource<std::vector<T>>(m_vecSink);
    p_vec->push_back(getSinkResource<T>(m_valueSink));
    setSourceResource(m_outSrc, p_vec);
  }

 private:
  const u32 m_vecSink;
  const u32 m_valueSink;
  const u32 m_outSrc;
};

}  // namespace sge
