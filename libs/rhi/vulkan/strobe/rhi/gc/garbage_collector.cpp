#include "strobe/rhi/gc/garbage_collector.hpp"
#include "strobe/rhi/gc/garbage_collector_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

void GarbageCollector::pin(void *handle) noexcept {
  pin_void_handle<GarbageCollectorImpl>(handle);
}

void GarbageCollector::unpin(void *handle) noexcept {
  unpin_void_handle<GarbageCollectorImpl>(handle);
}

void GarbageCollector::request_commit(Timepoint timepoint) noexcept {
  auto *impl = void_handle_ptr<GarbageCollectorImpl>(m_handle);
  impl->request_commit(timepoint);
}

void GarbageCollector::retire(Timepoint timepoint) noexcept {
  auto *impl = void_handle_ptr<GarbageCollectorImpl>(m_handle);
  impl->retire(timepoint);
}

void GarbageCollector::retire(Timepoint timepoint,
                              span<const CommandBuffer> cmds) {
  auto *impl = void_handle_ptr<GarbageCollectorImpl>(m_handle);
  impl->retire(timepoint, cmds);
}

void GarbageCollector::retire(Timepoint timepoint,
                              span<const BinarySemaphore> sems) {
  auto *impl = void_handle_ptr<GarbageCollectorImpl>(m_handle);
  impl->retire(timepoint, sems);
}

void GarbageCollector::retire(Fence fence) noexcept {
  auto *impl = void_handle_ptr<GarbageCollectorImpl>(m_handle);
  impl->retire(fence);
}

} // namespace strobe::rhi
