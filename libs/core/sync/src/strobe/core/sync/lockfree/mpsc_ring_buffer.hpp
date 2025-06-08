#pragma once

#include <atomic>
#include <optional>

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/align.hpp"

namespace strobe::sync {

namespace details {

// Optimized LockFreeMPSCRingBuffer (Multiple-Producer, Single-Consumer)
template <typename T> class LockFreeMPSCRingBufferBase {
protected:
  LockFreeMPSCRingBufferBase(T *buffer, std::size_t capacity)
      : m_head(0), m_tail(0), m_buffer(buffer), m_capacity(capacity) {}

public:
  // Enqueue (Multiple Producers)
  bool enqueue(const T &value) {
    size_t current_tail;
    size_t next_tail;

    do {
      current_tail = m_tail.load(std::memory_order_relaxed);
      next_tail = (current_tail + 1) % (m_capacity + 1);
      if (next_tail == m_head.load(std::memory_order_acquire)) {
        return false; // Buffer is full
      }
    } while (!m_tail.compare_exchange_weak(current_tail, next_tail));

    std::construct_at(m_buffer + current_tail, value);

    return true;
  }

  bool enqueue(T &&value) {
    size_t current_tail;
    size_t next_tail;
    do {
      current_tail = m_tail.load();
      next_tail = (current_tail + 1) % (m_capacity + 1);
      if (next_tail == m_head.load()) {
        return false; // Buffer is full
      }
    } while (!m_tail.compare_exchange_weak(current_tail, next_tail));

    std::construct_at(m_buffer + current_tail, std::move(value));
    return true;
  }

  // Dequeue (Single Consumer)
  std::optional<T> dequeue() {
    size_t current_head = m_head.load(std::memory_order_relaxed);

    if (current_head == m_tail.load(std::memory_order_acquire)) {
      return std::nullopt; // Buffer is empty
    }

    T *element_ptr = m_buffer + current_head;
    T value = std::move(*element_ptr);
    std::destroy_at(element_ptr);

    m_head.store((current_head + 1) % (m_capacity + 1),
                 std::memory_order_release);
    return value;
  }

  // Clear the buffer (Single Consumer)
  void clear() {
    while (dequeue().has_value())
      ;
  }

  // Capacity Information (Constant)
  std::size_t capacity() const { return m_capacity; }

protected:
  static constexpr size_t CacheLineSize = 64;

  alignas(CacheLineSize) std::atomic<size_t> m_head;
  alignas(CacheLineSize) std::atomic<size_t> m_tail;
  T *m_buffer;
  std::size_t m_capacity;
};

} // namespace details

template <typename T, std::size_t Capacity>
class InplaceLockFreeMPSCRingBuffer
    : public details::LockFreeMPSCRingBufferBase<T> {
public:
  InplaceLockFreeMPSCRingBuffer()
      : details::LockFreeMPSCRingBufferBase<T>(reinterpret_cast<T *>(m_storage),
                                               Capacity) {}

  ~InplaceLockFreeMPSCRingBuffer() { this->clear(); }

private:
  alignas(alignof(T)) std::byte m_storage[(Capacity + 1) * sizeof(T)];
};

template <typename T, Allocator A>
class LockFreeMPSCRingBuffer : public details::LockFreeMPSCRingBufferBase<T> {
  using ATraits = AllocatorTraits<A>;

public:
  LockFreeMPSCRingBuffer(std::size_t capacity, A alloc = {})
      : details::LockFreeMPSCRingBufferBase<T>(
            ATraits::template allocate<T>(alloc, capacity), capacity),
        m_allocator(std::move(alloc)) {}

  ~LockFreeMPSCRingBuffer() {
    this->clear();
    ATraits::template deallocate<T>(m_allocator, this->m_buffer,
                                    this->m_capacity);
  }

private:
  [[no_unique_address]] A m_allocator;
};

template <typename T>
class FlexibleLockFreeMPSCRingBuffer
    : public details::LockFreeMPSCRingBufferBase<T> {
public:
  FlexibleLockFreeMPSCRingBuffer(std::size_t flexibleBufferSize)
      : details::LockFreeMPSCRingBufferBase<T>(m_storage,
                                               flexibleBufferSize - 1) {}
  ~FlexibleLockFreeMPSCRingBuffer() { this->clear(); }

  FlexibleLockFreeMPSCRingBuffer(const FlexibleLockFreeMPSCRingBuffer &) =
      delete;
  FlexibleLockFreeMPSCRingBuffer &
  operator=(const FlexibleLockFreeMPSCRingBuffer &) = delete;
  FlexibleLockFreeMPSCRingBuffer(FlexibleLockFreeMPSCRingBuffer &&) = delete;
  FlexibleLockFreeMPSCRingBuffer &
  operator=(FlexibleLockFreeMPSCRingBuffer &&) = delete;

  static constexpr std::size_t requiredByteSize(std::size_t capacity) {
    // compute aligned corrected flexible size.
    std::size_t offset = sizeof(FlexibleLockFreeMPSCRingBuffer<T>);
    offset = memory::align_up(offset, alignof(T));
    offset = offset + sizeof(T) * capacity;
    return offset;
  }

  std::size_t byteSize() const { return requiredByteSize(this->m_capacity); }

  std::size_t bufferSize() const { return this->m_capacity + 1; }

private:
  T m_storage[0]; // flexible array layout!
};

} // namespace strobe::sync
