#pragma once

#include <atomic>
#include <optional>

namespace strobe {

// Optimized LockFreeSPSCRingBuffer (Single-Producer, Single-Consumer)
template <typename T, std::size_t Capacity>
class LockFreeSPSCRingBuffer {
  static_assert(Capacity > 0,
                "Capacity must be greater than 0 for a ring buffer.");

 public:
  LockFreeSPSCRingBuffer() : m_head(0), m_tail(0) {}

  ~LockFreeSPSCRingBuffer() {
    clear();  // Ensure all objects are destroyed
  }

  // Enqueue (Single Producer)
  bool enqueue(const T& value) {
    size_t current_tail = m_tail.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % (Capacity + 1);

    if (next_tail == m_head.load(std::memory_order_acquire)) {
      return false;  // Buffer is full
    }

    std::construct_at(
        reinterpret_cast<T*>(m_buffer + (current_tail * sizeof(T))), value);
    m_tail.store(next_tail, std::memory_order_release);
    return true;
  }

  bool enqueue(T&& value) {
    size_t current_tail = m_tail.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % (Capacity + 1);

    if (next_tail == m_head.load(std::memory_order_acquire)) {
      return false;  // Buffer is full
    }

    std::construct_at(
        reinterpret_cast<T*>(m_buffer + (current_tail * sizeof(T))),
        std::move(value));
    m_tail.store(next_tail, std::memory_order_release);
    return true;
  }

  // Dequeue (Single Consumer)
  std::optional<T> dequeue() {
    size_t current_head = m_head.load(std::memory_order_relaxed);

    if (current_head == m_tail.load(std::memory_order_acquire)) {
      return std::nullopt;  // Buffer is empty
    }

    // Read and destroy the value
    T* obj_ptr = reinterpret_cast<T*>(m_buffer + (current_head * sizeof(T)));
    T value = std::move(*obj_ptr);
    std::destroy_at(obj_ptr);

    m_head.store((current_head + 1) % (Capacity + 1),
                 std::memory_order_release);
    return value;
  }

  // Clear (Single Consumer)
  void clear() { while (dequeue().has_value()); }

  // Capacity (Logical capacity defined by the user)
  constexpr std::size_t capacity() const { return Capacity; }

 private:
  static constexpr size_t CacheLineSize = 64;
  alignas(CacheLineSize) std::atomic<size_t> m_head;
  alignas(alignof(T)) std::byte m_buffer[(Capacity + 1) * sizeof(T)];
  alignas(CacheLineSize) std::atomic<size_t> m_tail;
};

}
