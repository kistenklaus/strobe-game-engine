#pragma once
#include "renderer/RenderPass.hpp"

namespace sge {

template <typename T>
class WrapWithVectorPass : public RenderPass {
 public:
  WrapWithVectorPass(RendererBackend* renderer, const std::string name)
      : RenderPass(renderer, name),
        m_vec(std::vector<T>(0)),
        m_sink(registerSink<T>("value")),
        m_src(registerSource<std::vector<T>>("vector")) {}

  void execute() override {
    if (m_vec.size() == 0) {
      m_vec.push_back(*getSinkResource<T>(m_sink));
    } else {
      m_vec.resize(1);
      m_vec[0] = *getSinkResource<T>(m_sink);
    }
    setSourceResource(m_src, &m_vec);
  }

  u32 getSingleSink() const { return m_sink; }
  u32 getVectorSource() const { return m_src; }

 private:
  std::vector<T> m_vec;
  const u32 m_sink;
  const u32 m_src;
};

}  // namespace sge
