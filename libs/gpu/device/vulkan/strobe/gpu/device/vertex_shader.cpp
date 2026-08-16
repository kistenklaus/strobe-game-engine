#include "strobe/gpu/device/vertex_shader.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/shader_object_impl.hpp"

namespace strobe::gpu {

VertexShader::VertexShader(const VertexShader &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<ShaderObjectImpl>(m_handle);
  }
}

VertexShader::VertexShader(VertexShader &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

VertexShader &VertexShader::operator=(const VertexShader &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ShaderObjectImpl>(o.m_handle);
  }
  unpin_void_handle<ShaderObjectImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

VertexShader &VertexShader::operator=(VertexShader &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ShaderObjectImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

VertexShader::~VertexShader() noexcept {
  unpin_void_handle<ShaderObjectImpl>(m_handle);
}

} // namespace strobe::gpu
