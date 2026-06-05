#pragma once

#include <atomic>
#include <limits>

namespace strobe::ecs {

struct cmd_domain;

struct null_cmd_index_t {};
static constexpr null_cmd_index_t null_cmd_index{};

struct cmd_index {
  friend struct cmd_domain;

public:
  cmd_index(null_cmd_index_t) : m_index(invalid_index) {}
  cmd_index() : m_index(invalid_index) {}

  auto operator<=>(const cmd_index &) const = default;

  explicit operator uint32_t() const noexcept {
    return m_index;
  }

private:
  static constexpr uint32_t invalid_index =
      std::numeric_limits<uint32_t>::max();
  explicit cmd_index(uint32_t index) : m_index(index) {}
  uint32_t m_index;
};

struct cmd_domain {
public:
  // thread-safe
  cmd_index next() noexcept {
    return cmd_index{m_counter.fetch_add(1, std::memory_order_relaxed)};
  }

  // submission-only
  void reset() noexcept { m_counter.store(0, std::memory_order_release); }

private:
  std::atomic<uint32_t> m_counter;
};

} // namespace strobe::ecs
