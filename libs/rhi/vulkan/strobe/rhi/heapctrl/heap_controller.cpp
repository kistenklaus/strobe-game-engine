#include "strobe/rhi/heapctrl/heap_controller.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heapctrl/heap_controller_impl.hpp"

namespace strobe::rhi {

void HeapController::pin(void *handle) noexcept {
  pin_void_handle<HeapControllerImpl>(handle);
}

void HeapController::unpin(void *handle) noexcept {
  unpin_void_handle<HeapControllerImpl>(handle);
}

ResourceDescriptor HeapController::create_resource_descriptor(
    const ResourceDescriptorInfo &info) noexcept {
  auto *impl = void_handle_ptr<HeapControllerImpl>(m_handle);
  return impl->create_resource_descriptor(info);
}

ResourceDescriptorArray HeapController::create_resource_descriptor_array(
    span<const ResourceDescriptorInfo> infos) noexcept {
  auto *impl = void_handle_ptr<HeapControllerImpl>(m_handle);
  return impl->create_resource_descriptor_array(infos);
}

} // namespace strobe::rhi
