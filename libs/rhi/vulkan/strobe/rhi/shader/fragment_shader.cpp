#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/shader/shader_object_impl.hpp"

namespace strobe::rhi {

void FragmentShader::pin(void *handle) noexcept {
  pin_void_handle<ShaderObjectImpl>(handle);
}
void FragmentShader::unpin(void *handle) noexcept {
  unpin_void_handle<ShaderObjectImpl>(handle);
}

} // namespace strobe::rhi
