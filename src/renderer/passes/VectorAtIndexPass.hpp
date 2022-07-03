#pragma once
#include "renderer/RenderPass.hpp"

namespace sge::vulkan {

template <typename T>
class VectorAtIndexPass : public RenderPass {
 public:
  VectorAtIndexPass(RendererBackend* renderer, const std::string name)
      : RenderPass(renderer, name),
        m_vectorSink(registerSink<std::vector<T>>("vector")),
        m_indexSink(registerSink<u32>("index")),
        m_valueSource(registerSource<T>("value")) {}

  void execute() override {
    const std::vector<T>* p_vector =
        getSinkResource<std::vector<T>>(m_vectorSink);
    const u32* p_index = getSinkResource<u32>(m_indexSink);
    setSourceResource(m_valueSource, &(*p_vector)[(*p_index)]);
  }

 private:
  const u32 m_vectorSink;
  const u32 m_indexSink;
  const u32 m_valueSource;
};

}  // namespace sge::vulkan
