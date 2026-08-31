#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/binary_semaphore.hpp"
#include "strobe/rhi/sync/timepoint.hpp"
namespace strobe::rhi {

class GarbageCollector : Object<GarbageCollector> {
public:
  explicit GarbageCollector(void *handle) noexcept : Object(handle) {}
  GarbageCollector() noexcept : Object(nullptr) {}
  GarbageCollector(const GarbageCollector &) noexcept;
  GarbageCollector(GarbageCollector &&) noexcept;
  GarbageCollector &operator=(const GarbageCollector &) noexcept;
  GarbageCollector &operator=(GarbageCollector &&) noexcept;
  ~GarbageCollector() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const GarbageCollector &lhs,
                         const GarbageCollector &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const GarbageCollector &lhs,
                         const GarbageCollector &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }

  void retire(Timepoint timepoint, span<const CommandBuffer> cmds);
  void retire(Timepoint timepoint, span<const BinarySemaphore> sems);
};

} // namespace strobe::rhi
