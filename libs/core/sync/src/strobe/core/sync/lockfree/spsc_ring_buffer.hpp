#pragma once

#include <atomic>
#include <optional>
#include <strobe/memory.hpp>

#include "strobe/core/memory/AllocatorTraits.hpp"

namespace strobe::sync {

namespace details {

template <typename T> class LockFreeSPSCRingBufferBase {
protected:
  LockFreeSPSCRingBufferBase(T *buffer, std::size_t capacity)
      : m_head(0), m_tail(0), m_buffer(buffer), m_capacity(capacity) {
    assert(buffer != nullptr);
  }

public:
  bool enqueue(const T &value) {
    size_t current_tail = m_tail.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % (m_capacity + 1);

    if (next_tail == m_head.load(std::memory_order_acquire)) {
      return false;
    }

    std::construct_at(m_buffer + current_tail, value);
    m_tail.store(next_tail, std::memory_order_release);
    return true;
  }

  bool enqueue(T &&value) {
    size_t current_tail = m_tail.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % (m_capacity + 1);

    if (next_tail == m_head.load(std::memory_order_acquire)) {
      return false;
    }

    std::construct_at(m_buffer + current_tail, std::move(value));
    m_tail.store(next_tail, std::memory_order_release);
    return true;
  }

  std::optional<T> dequeue() {
    size_t current_head = m_head.load(std::memory_order_relaxed);

    if (current_head == m_tail.load(std::memory_order_acquire)) {
      return std::nullopt;
    }

    T *obj_ptr = m_buffer + current_head;
    T value = std::move(*obj_ptr);
    std::destroy_at(obj_ptr);

    m_head.store((current_head + 1) % (m_capacity + 1),
                 std::memory_order_release);
    return value;
  }

  void clear() {
    while (dequeue().has_value())
      ;
  }

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
class InplaceLockFreeSPSCRingBuffer final
    : public details::LockFreeSPSCRingBufferBase<T> {
public:
  InplaceLockFreeSPSCRingBuffer()
      : details::LockFreeSPSCRingBufferBase<T>(reinterpret_cast<T *>(m_storage),
                                               Capacity) {}

  ~InplaceLockFreeSPSCRingBuffer() { this->clear(); }

  InplaceLockFreeSPSCRingBuffer(const InplaceLockFreeSPSCRingBuffer &o) =
      delete;
  InplaceLockFreeSPSCRingBuffer &
  operator=(const InplaceLockFreeSPSCRingBuffer &o) = delete;
  InplaceLockFreeSPSCRingBuffer(InplaceLockFreeSPSCRingBuffer &&o) = delete;
  InplaceLockFreeSPSCRingBuffer &
  operator=(InplaceLockFreeSPSCRingBuffer &&o) = delete;

private:
  alignas(alignof(T)) std::byte m_storage[(Capacity + 1) * sizeof(T)];
};

template <typename T, strobe::Allocator A>
class LockFreeSPSCRingBuffer final
    : public details::LockFreeSPSCRingBufferBase<T> {
  using ATraits = AllocatorTraits<A>;

public:
  LockFreeSPSCRingBuffer(std::size_t capacity, A allocator = {})
      : details::LockFreeSPSCRingBufferBase<T>(
            ATraits::template allocate<T>(allocator, capacity + 1), capacity),
        m_allocator(std::move(allocator)) {}
  ~LockFreeSPSCRingBuffer() {
    this->clear();
    ATraits::template deallocate<T>(m_allocator, this->m_buffer,
                                    this->m_capacity);
  }

private:
  [[no_unique_address]] A m_allocator;
};

template <typename T>
class FlexibleLockFreeSPSCRingBuffer final
    : public details::LockFreeSPSCRingBufferBase<T> {
public:
  FlexibleLockFreeSPSCRingBuffer(std::size_t flexibleBufferSize)
      : details::LockFreeSPSCRingBufferBase<T>(reinterpret_cast<T*>(m_storage), flexibleBufferSize) {}

  ~FlexibleLockFreeSPSCRingBuffer() { this->clear(); }

  FlexibleLockFreeSPSCRingBuffer(const FlexibleLockFreeSPSCRingBuffer &) =
      delete;
  FlexibleLockFreeSPSCRingBuffer &
  operator=(const FlexibleLockFreeSPSCRingBuffer &) = delete;
  FlexibleLockFreeSPSCRingBuffer(FlexibleLockFreeSPSCRingBuffer &&) = delete;
  FlexibleLockFreeSPSCRingBuffer &
  operator=(FlexibleLockFreeSPSCRingBuffer &&) = delete;

  static constexpr std::size_t requiredByteSize(std::size_t capacity) {
    // compute aligned corrected flexible size.
    std::size_t offset = sizeof(FlexibleLockFreeSPSCRingBuffer<T>);
    offset = memory::align_up(offset, alignof(T));
    offset = offset + sizeof(T) * capacity;
    return offset;
  }

  std::size_t byteSize() const { return requiredByteSize(this->m_capacity); }

  std::size_t bufferSize() const { return this->m_capacity + 1; }

private:
  alignas(T) std::byte m_storage[sizeof(T)];
};

} // namespace strobe::sync
