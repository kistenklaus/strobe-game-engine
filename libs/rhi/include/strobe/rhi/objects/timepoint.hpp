#pragma once

#include <cassert>
#include <cstdint>
#include <limits>

namespace strobe::rhi {

class Timepoint {
  friend class Timeline;
  friend struct TimelineImpl;
  friend class TimelineBarrier;

public:
  Timepoint() noexcept : m_handle(nullptr), m_serial{0} {}
  Timepoint(const Timepoint &) noexcept;
  Timepoint(Timepoint &&) noexcept;
  Timepoint &operator=(const Timepoint &) noexcept;
  Timepoint &operator=(Timepoint &&) noexcept;
  ~Timepoint() noexcept;

  explicit operator bool() const noexcept { return m_handle != nullptr; }
  friend bool operator==(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert_compatible(lhs, rhs);
    return lhs.m_serial == rhs.m_serial;
  }

  friend bool operator!=(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator<(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert_compatible(lhs, rhs);
    return lhs.m_serial < rhs.m_serial;
  }

  friend bool operator<=(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert_compatible(lhs, rhs);
    return lhs.m_serial <= rhs.m_serial;
  }

  friend bool operator>(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert_compatible(lhs, rhs);
    return lhs.m_serial > rhs.m_serial;
  }

  friend bool operator>=(const Timepoint &lhs, const Timepoint &rhs) noexcept {
    assert_compatible(lhs, rhs);
    return lhs.m_serial >= rhs.m_serial;
  }

  friend uint64_t operator-(const Timepoint &lhs,
                            const Timepoint &rhs) noexcept {
    assert_compatible(lhs, rhs);
    assert(lhs.m_serial >= rhs.m_serial &&
           "timepoint subtraction would underflow");

    return lhs.m_serial - rhs.m_serial;
  }

  friend Timepoint operator-(const Timepoint &lhs, uint64_t rhs) noexcept {
    assert_valid(lhs);
    assert(lhs.m_serial >= rhs && "timepoint subtraction would underflow");

    const uint64_t serial = lhs.m_serial - rhs;

    // Serial zero is always represented by the universal null timepoint.
    if (serial == 0) {
      return {};
    }

    return Timepoint{lhs.m_handle, serial};
  }

  friend Timepoint operator+(const Timepoint &lhs, uint64_t rhs) noexcept {
    assert_valid(lhs);

    if (lhs.m_handle == nullptr) {
      assert(rhs == 0 && "cannot advance a null timepoint without a timeline");
      return {};
    }

    assert(rhs <= std::numeric_limits<uint64_t>::max() - lhs.m_serial &&
           "timepoint addition would overflow");

    return Timepoint{lhs.m_handle, lhs.m_serial + rhs};
  }

  friend Timepoint operator&(const Timepoint &lhs,
                             const Timepoint &rhs) noexcept {
    assert_compatible(lhs, rhs);

    // This naturally handles valid/null and null/null.
    return lhs.m_serial >= rhs.m_serial ? lhs : rhs;
  }

  Timepoint &operator&=(const Timepoint &o) noexcept {
    assert_compatible(*this, o);

    if (o.m_serial > m_serial) {
      *this = o;
    }

    return *this;
  }

  bool wait(uint64_t timeout) const noexcept;
  bool poll() const noexcept;
  bool relaxed_poll() const noexcept;

private:
  static void
  assert_valid([[maybe_unused]] const Timepoint &timepoint) noexcept {
    assert(((timepoint.m_handle == nullptr) == (timepoint.m_serial == 0)) &&
           "null timepoints must have serial zero and timeline "
           "timepoints must have a nonzero serial");
  }

  static void
  assert_compatible([[maybe_unused]] const Timepoint &lhs,
                    [[maybe_unused]] const Timepoint &rhs) noexcept {
    assert_valid(lhs);
    assert_valid(rhs);

    assert((lhs.m_handle == nullptr || rhs.m_handle == nullptr ||
            lhs.m_handle == rhs.m_handle) &&
           "timepoints belong to different timelines");
  }

  explicit Timepoint(void *handle, uint64_t serial) noexcept;
  void *m_handle;
  uint64_t m_serial;
};

} // namespace strobe::rhi
