#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/shader/shader_object_impl.hpp"
#include "strobe/rhi/vulkan/debug_name.hpp"

namespace strobe::rhi {

void ComputeShader::pin(void *handle) noexcept {
  pin_void_handle<ShaderObjectImpl>(handle);
}
void ComputeShader::unpin(void *handle) noexcept {
  unpin_void_handle<ShaderObjectImpl>(handle);
}

void ComputeShader::set_name(const char *name) {
  assert(m_handle);
  auto *impl = void_handle_ptr<ShaderObjectImpl>(m_handle);
  vulkan::set_debug_name(impl->context.ctx(), impl->shader, name);
}

} // namespace strobe::rhi
