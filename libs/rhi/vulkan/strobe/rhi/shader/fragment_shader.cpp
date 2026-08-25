#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/shader/shader_object_impl.hpp"

namespace strobe::rhi {

FragmentShader::FragmentShader(const FragmentShader & o) noexcept 
  : m_handle(o.m_handle) {
    if (m_handle != nullptr) {
      pin_void_handle<ShaderObjectImpl>(m_handle);
    }
  }

FragmentShader::FragmentShader(FragmentShader && o) noexcept 
  : m_handle(std::exchange(o.m_handle, nullptr)){
}

FragmentShader &FragmentShader::operator=(const FragmentShader & o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ShaderObjectImpl>(o.m_handle);
  }
  unpin_void_handle<ShaderObjectImpl>(m_handle);;
  m_handle = o.m_handle;
  return *this;
}

FragmentShader &FragmentShader::operator=(FragmentShader && o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ShaderObjectImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

FragmentShader::~FragmentShader() noexcept {
  unpin_void_handle<ShaderObjectImpl>(m_handle);
}

} // namespace strobe::rhi
