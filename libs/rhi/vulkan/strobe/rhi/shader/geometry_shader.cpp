#include "strobe/rhi/objects/geometry_shader.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/shader/shader_object_impl.hpp"

namespace strobe::rhi {

void GeometryShader::pin(void * handle) noexcept {
  pin_void_handle<ShaderObjectImpl>(handle);
}

void GeometryShader::unpin(void * handle) noexcept {
  unpin_void_handle<ShaderObjectImpl>(handle);
}

} // namespace strobe::rhi
