#pragma once

#include <cassert>
#include <cstdint>
#include <limits>

    namespace strobe::rhi {

  /**
   * \ingroup rhi
   * \brief Device synchronization point.
   * Defined in header <strobe/rhi/rhi.hpp>
   * \code{.cpp}
   * class Timepoint;
   * \endcode
   *
   * Represents completion of asynchronous device work on a timeline and may be
   * waited on from the host or used as a dependency for subsequent device work.
   *
   * A null Timepoint represents the universal zero point and is considered
   * earlier than every non-null Timepoint.
   *
   * \attention 1. Non-null Timepoints may only be compared or combined when
   * they belong to the same timeline.
   */
  class Timepoint {
    friend class Timeline;
    friend struct TimelineImpl;
    friend class TimelineBarrier;

  public:
    /**
     * \brief Constructs null timepoint.
     *
     * Constructs the universal zero Timepoint.
     */
    Timepoint() noexcept : m_handle(nullptr), m_serial{0} {}

    /**
     * \brief copy-constructor
     */
    Timepoint(const Timepoint &o) noexcept;

    /**
     * \brief move-constructor
     */
    Timepoint(Timepoint &&o) noexcept;

    /**
     * \brief copy-assignment
     */
    Timepoint &operator=(const Timepoint &o) noexcept;

    /**
     * \brief move-assignment
     */
    Timepoint &operator=(Timepoint &&o) noexcept;

    /**
     * \brief destructor
     */
    ~Timepoint() noexcept;

    /**
     * \brief Checks timepoint validity.
     *
     * Tests whether this Timepoint references a timeline.
     *
     * \return true for a non-null Timepoint, otherwise false.
     */
    explicit operator bool() const noexcept { return m_handle != nullptr; }

    /**
     * \brief Compares timepoints.
     *
     * Tests whether two Timepoints refer to the same timeline position.
     *
     * \param lhs Left-hand Timepoint.
     * \param rhs Right-hand Timepoint.
     *
     * \return true if both Timepoints represent the same position.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     */
    friend bool operator==(const Timepoint &lhs,
                           const Timepoint &rhs) noexcept {
      assert_compatible(lhs, rhs);
      return lhs.m_serial == rhs.m_serial;
    }

    /**
     * \brief Compares timepoints.
     *
     * Tests whether two Timepoints represent different timeline positions.
     *
     * \param lhs Left-hand Timepoint.
     * \param rhs Right-hand Timepoint.
     *
     * \return true if the Timepoints differ.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     */
    friend bool operator!=(const Timepoint &lhs,
                           const Timepoint &rhs) noexcept {
      return !(lhs == rhs);
    }

    /**
     * \brief Orders timepoints.
     *
     * Tests whether \p lhs precedes \p rhs.
     *
     * \param lhs Left-hand Timepoint.
     * \param rhs Right-hand Timepoint.
     *
     * \return true if \p lhs precedes \p rhs.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     */
    friend bool operator<(const Timepoint &lhs, const Timepoint &rhs) noexcept {
      assert_compatible(lhs, rhs);
      return lhs.m_serial < rhs.m_serial;
    }

    /**
     * \brief Orders timepoints.
     *
     * Tests whether \p lhs precedes or equals \p rhs.
     *
     * \param lhs Left-hand Timepoint.
     * \param rhs Right-hand Timepoint.
     *
     * \return true if \p lhs precedes or equals \p rhs.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     */
    friend bool operator<=(const Timepoint &lhs,
                           const Timepoint &rhs) noexcept {
      assert_compatible(lhs, rhs);
      return lhs.m_serial <= rhs.m_serial;
    }

    /**
     * \brief Orders timepoints.
     *
     * Tests whether \p lhs follows \p rhs.
     *
     * \param lhs Left-hand Timepoint.
     * \param rhs Right-hand Timepoint.
     *
     * \return true if \p lhs follows \p rhs.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     */
    friend bool operator>(const Timepoint &lhs, const Timepoint &rhs) noexcept {
      assert_compatible(lhs, rhs);
      return lhs.m_serial > rhs.m_serial;
    }

    /**
     * \brief Orders timepoints.
     *
     * Tests whether \p lhs follows or equals \p rhs.
     *
     * \param lhs Left-hand Timepoint.
     * \param rhs Right-hand Timepoint.
     *
     * \return true if \p lhs follows or equals \p rhs.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     */
    friend bool operator>=(const Timepoint &lhs,
                           const Timepoint &rhs) noexcept {
      assert_compatible(lhs, rhs);
      return lhs.m_serial >= rhs.m_serial;
    }

    /**
     * \brief Computes timeline distance.
     *
     * Returns the number of timeline increments between two Timepoints.
     *
     * \param lhs Later Timepoint.
     * \param rhs Earlier Timepoint.
     *
     * \return Difference between the timeline serials.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     * \attention 2. \p lhs must not precede \p rhs.
     */
    friend uint64_t operator-(const Timepoint &lhs,
                              const Timepoint &rhs) noexcept {
      assert_compatible(lhs, rhs);
      assert(lhs.m_serial >= rhs.m_serial &&
             "timepoint subtraction would underflow");

      return lhs.m_serial - rhs.m_serial;
    }

    /**
     * \brief Rewinds timepoint.
     *
     * Returns a Timepoint \p rhs increments before \p lhs.
     *
     * \param lhs Source Timepoint.
     * \param rhs Number of timeline increments.
     *
     * \return Resulting Timepoint.
     *
     * \attention 1. \p rhs must not exceed the serial of \p lhs.
     */
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

    /**
     * \brief Advances timepoint.
     *
     * Returns a Timepoint \p rhs increments after \p lhs.
     *
     * \param lhs Source Timepoint.
     * \param rhs Number of timeline increments.
     *
     * \return Resulting Timepoint.
     *
     * \attention 1. A null Timepoint may only be advanced by zero.
     * \attention 2. The resulting serial must not overflow.
     */
    friend Timepoint operator+(const Timepoint &lhs, uint64_t rhs) noexcept {
      assert_valid(lhs);

      if (lhs.m_handle == nullptr) {
        assert(rhs == 0 &&
               "cannot advance a null timepoint without a timeline");
        return {};
      }

      assert(rhs <= std::numeric_limits<uint64_t>::max() - lhs.m_serial &&
             "timepoint addition would overflow");

      return Timepoint{lhs.m_handle, lhs.m_serial + rhs};
    }

    /**
     * \brief Combines timepoints.
     *
     * Returns the later of two compatible Timepoints.
     *
     * \param lhs Left-hand Timepoint.
     * \param rhs Right-hand Timepoint.
     *
     * \return Later Timepoint.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     */
    friend Timepoint operator&(const Timepoint &lhs,
                               const Timepoint &rhs) noexcept {
      assert_compatible(lhs, rhs);

      // This naturally handles valid/null and null/null.
      return lhs.m_serial >= rhs.m_serial ? lhs : rhs;
    }

    /**
     * \brief Combines timepoints.
     *
     * Replaces this Timepoint with the later compatible Timepoint.
     *
     * \param o Timepoint to combine.
     *
     * \return Reference to this Timepoint.
     *
     * \attention 1. Non-null Timepoints must belong to the same timeline.
     */
    Timepoint &operator&=(const Timepoint &o) noexcept {
      assert_compatible(*this, o);

      if (o.m_serial > m_serial) {
        *this = o;
      }

      return *this;
    }

    /**
     * \brief Waits for completion.
     *
     * Blocks the calling thread until this Timepoint completes or the timeout
     * expires.
     *
     * \param timeout Maximum wait duration.
     *
     * \return true if the Timepoint completed before the timeout.
     */
    bool wait(uint64_t timeout) const noexcept;

    /**
     * \brief Polls for completion.
     *
     * Queries whether this Timepoint has completed.
     *
     * \return true if the Timepoint has completed.
     */
    bool poll() const noexcept;

    /**
     * \brief Polls cached completion.
     *
     * Checks locally cached completion state without forcing a device query.
     *
     * \return true if the Timepoint is known to have completed.
     *
     * \note false does not necessarily mean that the device operation has not
     * completed.
     */
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
