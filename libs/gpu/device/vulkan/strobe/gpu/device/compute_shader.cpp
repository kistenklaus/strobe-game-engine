#include "strobe/gpu/device/compute_shader.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/device/shader_object_impl.hpp"
#include "strobe/gpu/vulkan/debug_name.hpp"

namespace strobe::gpu {

ComputeShader::ComputeShader(const ComputeShader & o) noexcept : m_handle(o.m_handle){
  if (m_handle) {
    pin_void_handle<ShaderObjectImpl>(m_handle);
  }
}

ComputeShader::ComputeShader(ComputeShader && o) noexcept : m_handle(std::exchange(o.m_handle, nullptr)) {

}

ComputeShader &ComputeShader::operator=(const ComputeShader &o ) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<ShaderObjectImpl>(m_handle);
  }
  unpin_void_handle<ShaderObjectImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

ComputeShader &ComputeShader::operator=(ComputeShader && o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<ShaderObjectImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

ComputeShader::~ComputeShader() noexcept {
  unpin_void_handle<ShaderObjectImpl>(m_handle);
}

void ComputeShader::set_name(const char *name) {
  assert(m_handle);
  auto* impl = void_handle_ptr<ShaderObjectImpl>(m_handle);
  vulkan::set_debug_name(impl->context.get(), impl->shader, name);
}

} // namespace strobe::gpu
