#pragma once

#include <atomic>
#include <cassert>
#include <cwchar>
#include <memory>
#include <optional>
#include <print>

#include "strobe/core/memory/AllocatorTraits.hpp"

namespace strobe {

namespace details {

template <typename T>
class RingBufferBase {
 protected:
  RingBufferBase(T* buffer, std::size_t capacity)
      : m_head(0),
        m_tail(0),
        m_size(0),
        m_capacity(capacity),
        m_buffer(buffer) {}

 public:
  virtual ~RingBufferBase() {}

  // Push an item to the back
  void push_back(const T& value) {
    assert(!full());
    std::construct_at(m_buffer + m_tail, value);
    m_tail = (m_tail + 1) % m_capacity;
    ++m_size;
  }

  void push_back(T&& value) {
    assert(!full());
    std::construct_at(m_buffer + m_tail, std::move(value));
    m_tail = (m_tail + 1) % m_capacity;
    ++m_size;
  }

  void pop_back() {
    assert(!empty());
    m_tail = (m_tail + m_capacity - 1) % m_capacity;
    std::destroy_at(m_buffer + m_tail);
    --m_size;
  }

  void push_front(const T& value) {
    assert(!full());
    m_head = (m_head + m_capacity - 1) % m_capacity;
    std::construct_at(m_buffer + m_head, value);
    ++m_size;
  }

  void push_front(T&& value) {
    assert(!full());
    m_head = (m_head + m_capacity - 1) % m_capacity;
    std::construct_at(m_buffer + m_head, std::move(value));
    ++m_size;
  }

  void pop_front() {
    assert(!empty());
    std::destroy_at(m_buffer + m_head);
    m_head = (m_head + 1) % m_capacity;
    --m_size;
  }

  T& front() {
    assert(!empty());
    return m_buffer[m_head];
  }

  T& back() {
    assert(!empty());
    return *(m_buffer + ((m_tail + m_capacity - 1) % m_capacity));
  }

  void clear() {
    while (!empty()) pop_front();
  }

  bool full() const { return m_size == m_capacity; }
  bool empty() const { return m_size == 0; }
  std::size_t size() const { return m_size; }
  size_t capacity() const { return m_capacity; }

  // queue interface

  bool enqueue(const T& value) {
    if (full()) {
      return false;
    }
    push_back(value);
    return true;
  }
  bool enqueue(T&& value) {
    if (full()) {
      return false;
    }
    push_back(std::move(value));
    return true;
  }
  std::optional<T> dequeue() {
    if (empty()) {
      return std::nullopt;
    }
    T v = std::move(front());
    pop_front();
    return v;
  }

 protected:
  std::size_t m_head;
  std::size_t m_tail;
  std::size_t m_size;
  std::size_t m_capacity;
  T* m_buffer;
};

}  // namespace details

template <typename T, std::size_t Capacity>
class InplaceRingBuffer final : public details::RingBufferBase<T> {
 public:
  InplaceRingBuffer()
      : details::RingBufferBase<T>(reinterpret_cast<T*>(m_storage), Capacity) {}

  ~InplaceRingBuffer() override { this->clear(); }

  InplaceRingBuffer(const InplaceRingBuffer& o) = delete;
  InplaceRingBuffer& operator=(const InplaceRingBuffer& o) = delete;
  InplaceRingBuffer(InplaceRingBuffer&& o) = delete;
  InplaceRingBuffer& operator=(InplaceRingBuffer&& o) = delete;

 private:
  alignas(alignof(T)) std::byte m_storage[(Capacity + 1) * sizeof(T)];
};

template <typename T, Allocator A>
class RingBuffer final : public details::RingBufferBase<T> {
  using ATraits = AllocatorTraits<A>;

 public:
  RingBuffer(std::size_t capacity, A allocator = {})
      : details::RingBufferBase<T>(
            ATraits::template allocate<T>(allocator, capacity), capacity),
        m_allocator(std::move(allocator)) {}

  ~RingBuffer() {
    ATraits::template deallocate<T>(m_allocator, this->m_buffer,
                                    this->m_capacity);
  }

  RingBuffer(const RingBuffer& o) = delete;
  RingBuffer& operator=(const RingBuffer& o) = delete;
  RingBuffer(RingBuffer&& o) = delete;
  RingBuffer& operator=(RingBuffer&& o) = delete;

 private:
  [[no_unique_address]] A m_allocator;
};

#if FALSE
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

// Optimized LockFreeMPSCRingBuffer (Multiple-Producer, Single-Consumer)
template <typename T, std::size_t Capacity>
class LockFreeMPSCRingBuffer {
 public:
  LockFreeMPSCRingBuffer() : m_head(0), m_tail(0) {}

  // Enqueue (Multiple Producers)
  bool enqueue(const T& value) {
    size_t current_tail;
    size_t next_tail;
    do {
      current_tail = m_tail.load(std::memory_order_relaxed);
      next_tail = (current_tail + 1) % BufferSize;
      if (next_tail == m_head.load(std::memory_order_acquire)) {
        return false;  // Buffer is full
      }
    } while (!m_tail.compare_exchange_weak(current_tail, next_tail,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));

    std::construct_at(reinterpret_cast<T*>(m_buffer + current_tail * sizeof(T)),
                      value);
    return true;
  }

  bool enqueue(T&& value) {
    size_t current_tail;
    size_t next_tail;
    do {
      current_tail = m_tail.load(std::memory_order_relaxed);
      next_tail = (current_tail + 1) % BufferSize;
      if (next_tail == m_head.load(std::memory_order_acquire)) {
        return false;  // Buffer is full
      }
    } while (!m_tail.compare_exchange_weak(current_tail, next_tail,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));

    std::construct_at(reinterpret_cast<T*>(m_buffer + current_tail * sizeof(T)),
                      std::move(value));
    return true;
  }

  // Dequeue (Single Consumer)
  std::optional<T> dequeue() {
    size_t current_head = m_head.load(std::memory_order_relaxed);

    if (current_head == m_tail.load(std::memory_order_acquire)) {
      return std::nullopt;  // Buffer is empty
    }

    T* element_ptr = reinterpret_cast<T*>(m_buffer + current_head * sizeof(T));
    T value = std::move(*element_ptr);
    std::destroy_at(element_ptr);

    m_head.store((current_head + 1) % BufferSize, std::memory_order_release);
    return value;
  }

  // Clear the buffer (Single Consumer)
  void clear() { while (dequeue().has_value()); }

  // Capacity Information (Constant)
  constexpr std::size_t capacity() const { return Capacity; }

 private:
  static constexpr size_t BufferSize =
      Capacity + 1;  // Adjusted for one slot for full/empty distinction
  static constexpr size_t CacheLineSize = 64;

  alignas(CacheLineSize) std::atomic<size_t> m_head;
  alignas(CacheLineSize) std::atomic<size_t> m_tail;
  alignas(alignof(T)) std::byte m_buffer[BufferSize * sizeof(T)];
};
#endif

}  // namespace strobe
   //
