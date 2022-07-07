#pragma once
#include "renderer/RenderPass.hpp"

namespace sge::vulkan {

template <typename T>
class VectorAtIndexPass : public RenderPass {
 public:
  VectorAtIndexPass(RendererBackend* renderer, const std::string name)
      : RenderPass(renderer, name),
        m_vectorSink(sink<std::vector<T>>("vector")),
        // m_vectorSink(registerSink<std::vector<T>>("vector")),
        m_indexSink(sink<u32>("index")),
        m_valueSource(source<T>("value")) {
    registerSink(&m_vectorSink);
    registerSink(&m_indexSink);
    registerSource(&m_valueSource);
  }

  void execute() override {
    T& value = (*m_vectorSink)[*m_indexSink];
    m_valueSource.set(&value);
  }

 private:
  sink<std::vector<T>> m_vectorSink;
  sink<u32> m_indexSink;
  source<T> m_valueSource;
};

}  // namespace sge::vulkan
