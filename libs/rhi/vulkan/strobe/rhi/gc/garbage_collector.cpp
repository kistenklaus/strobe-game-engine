#include "strobe/rhi/gc/garbage_collector.hpp"
#include "strobe/rhi/gc/garbage_collector_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

GarbageCollector::GarbageCollector(const GarbageCollector &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<GarbageCollectorImpl>(m_handle);
  }
}

GarbageCollector::GarbageCollector(GarbageCollector &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

GarbageCollector &
GarbageCollector::operator=(const GarbageCollector &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<GarbageCollectorImpl>(o.m_handle);
  }
  unpin_void_handle<GarbageCollectorImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

GarbageCollector &GarbageCollector::operator=(GarbageCollector &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<GarbageCollectorImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

GarbageCollector::~GarbageCollector() noexcept {
  unpin_void_handle<GarbageCollectorImpl>(m_handle);
}

void GarbageCollector::retire(Timepoint timepoint,
                              span<const CommandBuffer> cmds) {
  auto *impl = void_handle_ptr<GarbageCollectorImpl>(m_handle);
  impl->retire(timepoint, cmds);
}

} // namespace strobe::rhi
