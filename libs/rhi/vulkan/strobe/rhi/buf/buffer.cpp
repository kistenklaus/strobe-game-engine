#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/debug_name.hpp"
#include <vulkan/vulkan_core.h>

namespace strobe::rhi {

void Buffer::pin(void *handle) noexcept { pin_void_handle<BufferImpl>(handle); }
void Buffer::unpin(void *handle) noexcept {
  unpin_void_handle<BufferImpl>(handle);
}

uint64_t Buffer::size() const noexcept {
  if (m_handle == nullptr) {
    return 0;
  }
  auto *impl = void_handle_ptr<BufferImpl>(m_handle);
  return impl->size;
}

void *Buffer::ptr() const {
  auto *impl = void_handle_ptr<BufferImpl>(m_handle);
  impl->commit();
  return impl->allocation.map();
}

void Buffer::set_name(const char *name) const noexcept {
  assert(m_handle);
  auto *impl = void_handle_ptr<BufferImpl>(m_handle);
  vulkan::set_debug_name(impl->context.ctx(), impl->buffer, name);
}

} // namespace strobe::rhi
