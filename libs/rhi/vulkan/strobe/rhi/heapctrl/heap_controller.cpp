#include "strobe/rhi/heapctrl/heap_controller.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heapctrl/heap_controller_impl.hpp"

namespace strobe::rhi {

HeapController::HeapController(const HeapController &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<HeapControllerImpl>(m_handle);
  }
}

HeapController::HeapController(HeapController &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

HeapController &HeapController::operator=(const HeapController &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<HeapControllerImpl>(o.m_handle);
  }
  unpin_void_handle<HeapControllerImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

HeapController &HeapController::operator=(HeapController &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<HeapControllerImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

HeapController::~HeapController() noexcept {
  unpin_void_handle<HeapControllerImpl>(m_handle);
}

BufferDescriptor HeapController::create_buffer_descriptor(
    const BufferDescriptorInfo &info) noexcept {
  auto *impl = void_handle_ptr<HeapControllerImpl>(m_handle);
  return impl->create_buffer_descriptor(info);
}

BufferDescriptorArray HeapController::create_buffer_descriptor_array(
    span<const BufferDescriptorInfo> infos) noexcept {
  auto *impl = void_handle_ptr<HeapControllerImpl>(m_handle);
  return impl->create_buffer_descriptor_array(infos);
}

} // namespace strobe::rhi
