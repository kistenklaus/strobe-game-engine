#pragma once

#include <cstdint>
#include <limits>

namespace strobe::rhi {

class TimelineSemaphore {
  friend class Device;
  friend class Queue;

public:
  TimelineSemaphore() noexcept : m_handle(nullptr) {}
  TimelineSemaphore(const TimelineSemaphore &) noexcept;
  TimelineSemaphore(TimelineSemaphore &&) noexcept;
  TimelineSemaphore &operator=(const TimelineSemaphore &) noexcept;
  TimelineSemaphore &operator=(TimelineSemaphore &&) noexcept;
  ~TimelineSemaphore() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  uint64_t count() const;

  void signal(uint64_t value);

  bool wait(uint64_t value,
            uint64_t timeout = std::numeric_limits<uint64_t>::max());

  explicit TimelineSemaphore(void *handle) noexcept : m_handle(handle) {}

private:
  void *m_handle;
};

} // namespace strobe::rhi
