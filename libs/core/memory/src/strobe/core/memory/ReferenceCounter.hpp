#pragma once

#include <atomic>
#include <concepts>
namespace strobe::memory {

template <std::unsigned_integral T = std::size_t>
class ReferenceCounter {
 public:
  ReferenceCounter() : m_counter(1) {}
  ReferenceCounter(T inital) : m_counter(inital) {}

  void reset() { m_counter.store(1); }

  bool inc() {
    auto current = m_counter.load();
    while (current > 0 &&
           !m_counter.compare_exchange_weak(current, current + 1));
    return current > 0;
  }

  void resurrect() { 
    m_counter.fetch_add(1, std::memory_order_release); 
  }

  bool dec() { return m_counter.fetch_sub(1) == 1; }

  [[deprecated(
      "Unclear semantics. Use isZero if possible. Might also clash with "
      "waitless designs.")]]
  T useCount() const {
    return m_counter.load();
  }

  bool isZero() const { return m_counter.load() == 0; }

 private:
  std::atomic<T> m_counter;
};

}  // namespace strobe::memory
