#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/sync/timepoint.hpp"
#include "strobe/rhi/vulkan/timeline_semaphore.hpp"

namespace strobe::rhi {

class Timeline : public Object<Timeline> {
public:
  explicit Timeline(void *handle) noexcept : Object(handle) {}
  Timeline() noexcept : Object(nullptr) {}
  Timeline(const Timeline &) noexcept;
  Timeline(Timeline &&) noexcept;
  Timeline &operator=(const Timeline &) noexcept;
  Timeline &operator=(Timeline &&) noexcept;
  ~Timeline() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const Timeline &lhs, const Timeline &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const Timeline &lhs, const Timeline &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

  static void notify(Timepoint timepoint) noexcept;
  void notify(uint64_t serial) noexcept;

  Timepoint now() noexcept;
  bool contains(Timepoint timepoint) const noexcept;
  Timepoint epoch() const noexcept;
  void complete(Timepoint timepoint) noexcept;
  Timepoint advance() noexcept;
  uint64_t serial() const noexcept;
  uint64_t completed_serial() const noexcept;
  vulkan::TimelineSemaphore timelineSemaphore() const noexcept;

  void set_commit_callback(void *pUserData,
                           void (*commit)(void *, Timepoint)) noexcept;
  void clear_callback() noexcept;

  Timepoint from_serial(uint64_t serial) noexcept ;
};

} // namespace strobe::rhi
