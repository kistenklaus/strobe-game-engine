#pragma once

#include <cassert>
#include <cstdint>

namespace strobe::rhi {

class Timepoint {
  friend class Timeline;
  friend class TimelineBarrier;

public:
  Timepoint() noexcept : m_handle(nullptr), m_serial{0} {}
  Timepoint(const Timepoint &) noexcept = default;
  Timepoint(Timepoint &&) noexcept = default;
  Timepoint &operator=(const Timepoint &) noexcept = default;
  Timepoint &operator=(Timepoint &&) noexcept = default;
  ~Timepoint() noexcept = default;
  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert(lhs.m_handle == rhs.m_handle &&
           "cannot compare two timepoints from different timelines");
    return lhs.m_serial == rhs.m_serial;
  }
  friend bool operator!=(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert(lhs.m_handle == rhs.m_handle &&
           "cannot compare two timepoints from different timelines");
    return lhs.m_serial != rhs.m_serial;
  }
  friend bool operator<(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert(lhs.m_handle == rhs.m_handle &&
           "cannot compare two timepoints from different timelines");
    return lhs.m_serial < rhs.m_serial;
  }
  friend bool operator<=(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert(lhs.m_handle == rhs.m_handle &&
           "cannot compare two timepoints from different timelines");
    return lhs.m_serial <= rhs.m_serial;
  }
  friend bool operator>(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert(lhs.m_handle == rhs.m_handle &&
           "cannot compare two timepoints from different timelines");
    return lhs.m_serial > rhs.m_serial;
  }
  friend bool operator>=(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert(lhs.m_handle == rhs.m_handle &&
           "cannot compare two timepoints from different timelines");
    return lhs.m_serial >= rhs.m_serial;
  }
  friend uint64_t operator-(const Timepoint &lhs,
                            const Timepoint &rhs) noexcept {
    assert(lhs.m_handle == rhs.m_handle &&
           "cannot subtract two timepoints from different timelines");
    assert(lhs.m_serial >= rhs.m_serial &&
           "underflow of timepoints subtraction is well defined!");
    return lhs.m_serial - rhs.m_serial;
  }
  friend Timepoint operator-(const Timepoint &lhs, uint64_t rhs) noexcept {
    assert(lhs.m_serial >= rhs &&
           "underflow of timepoints subtraction is well defined!");
    return Timepoint{lhs.m_handle, lhs.m_serial - rhs};
  }
  friend Timepoint operator+(const Timepoint &lhs, uint64_t rhs) noexcept {
    return Timepoint{lhs.m_handle, lhs.m_serial + rhs};
  }

  bool wait(uint64_t timeout) const noexcept;
  bool poll() const noexcept;
  bool relaxed_poll() const noexcept;

private:
  explicit Timepoint(void *handle, uint64_t serial) noexcept
      : m_handle(handle), m_serial(serial) {}
  void *m_handle;
  uint64_t m_serial;
};

} // namespace strobe::rhi
