#pragma once

#include "strobe/gpu/vulkan/queue.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <limits>
#include <mutex>

namespace strobe::gpu {

struct NativeQueue {
  static constexpr uint32_t EXCLUSIVE = std::numeric_limits<uint32_t>::max();
  [[nodiscard]] bool try_acquire(bool exclusive) {
    if (exclusive) {
      uint32_t expected = 0;
      return users.compare_exchange_strong(expected, EXCLUSIVE,
                                           std::memory_order_relaxed,
                                           std::memory_order_relaxed);
    }
    uint32_t current = users.load(std::memory_order_relaxed);
    while(true) {
      if (current == EXCLUSIVE) {
        return false;
      }
      if (current == EXCLUSIVE - 1) {
        return false;
      }
      if (users.compare_exchange_weak(current, current + 1,
                                      std::memory_order_relaxed,
                                      std::memory_order_relaxed)) {
        return true;
      }
    }
  }

  void release(bool exclusive) noexcept {
    if (exclusive) {
      uint32_t expected = EXCLUSIVE;
      [[maybe_unused]] const bool success = users.compare_exchange_strong(
          expected, 0, std::memory_order_relaxed, std::memory_order_relaxed);
      assert(success);
      return;
    }
    [[maybe_unused]] const uint32_t previous = users.fetch_sub(1, std::memory_order_relaxed);
    assert(previous != 0);
    assert(previous != EXCLUSIVE);
  }

  [[nodiscard]] uint32_t user_count() const {
    const uint32_t value = users.load(std::memory_order_relaxed);

    return value == EXCLUSIVE ? 1 : value;
  }

  [[nodiscard]] bool exclusively_owned() const {
    return users.load(std::memory_order_relaxed) == EXCLUSIVE;
  }

  [[nodiscard]] bool available() const { return static_cast<bool>(queue); }

  vulkan::Queue queue;
  std::mutex mutex;

  vulkan::TimelineSemaphore timeline;
  std::atomic<uint64_t> nextTimelineValue{0};

private:
  // 0         -> unused
  // 1..N      -> shared by N logical queues
  // EXCLUSIVE -> exclusively owned by one logical queue
  std::atomic<uint32_t> users{0};
};

} // namespace strobe::gpu
