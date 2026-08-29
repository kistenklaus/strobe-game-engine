#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <strobe/memory.hpp>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T, Allocator A = strobe::Mallocator> class VectorDeque {
  using ATraits = AllocatorTraits<A>;

public:
  using value_type = T;
  using allocator_type = A;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = typename ATraits::template pointer<value_type>;
  using const_pointer = typename ATraits::template const_pointer<value_type>;

  explicit VectorDeque(const A &allocator = {}) : m_allocator(allocator) {}

  explicit VectorDeque(size_type capacity, const A &allocator = {})
      : m_allocator(allocator) {
    reserve(capacity);
  }

  ~VectorDeque() { reset(); }

  VectorDeque(const VectorDeque &other)
      : m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)) {
    if (other.empty()) {
      return;
    }

    auto [buffer, capacity] = allocate_storage(other.size());
    m_buffer = buffer;
    m_capacity = capacity;
    copy_construct_from(other);
  }

  VectorDeque &operator=(const VectorDeque &other) {
    if (this == &other) {
      return *this;
    }

    const bool equalAllocator =
        strobe::alloc_equals(m_allocator, other.m_allocator);

    if constexpr (ATraits::propagate_on_container_copy_assignment) {
      if (!equalAllocator) {
        reset();
      }

      m_allocator = other.m_allocator;
    }

    clear();

    if (other.size() > m_capacity) {
      deallocate_storage(m_buffer, m_capacity);
      m_buffer = nullptr;
      m_capacity = 0;

      auto [buffer, capacity] = allocate_storage(other.size());
      m_buffer = buffer;
      m_capacity = capacity;
    }

    copy_construct_from(other);
    return *this;
  }

  VectorDeque(VectorDeque &&other) noexcept
      : m_capacity(std::exchange(other.m_capacity, 0)),
        m_size(std::exchange(other.m_size, 0)),
        m_begin(std::exchange(other.m_begin, 0)),
        m_buffer(std::exchange(other.m_buffer, nullptr)),
        m_allocator(std::move(other.m_allocator)) {}

  VectorDeque &operator=(VectorDeque &&other) noexcept {
    if (this == &other) {
      return *this;
    }

    const bool equalAllocator =
        strobe::alloc_equals(m_allocator, other.m_allocator);

    if constexpr (ATraits::propagate_on_container_move_assignment) {
      reset();
      m_allocator = std::move(other.m_allocator);
      steal_storage(other);
      return *this;
    }

    if (equalAllocator) {
      reset();
      steal_storage(other);
      return *this;
    }

    clear();
    reserve(other.size());
    move_construct_from(other);
    other.clear();
    return *this;
  }

  [[nodiscard]] bool full() const noexcept { return m_size == m_capacity; }

  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

  [[nodiscard]] size_type capacity() const noexcept { return m_capacity; }

  [[nodiscard]] size_type size() const noexcept { return m_size; }

  void reserve(size_type newCapacity) {
    if (newCapacity <= m_capacity) {
      return;
    }

    auto [newBuffer, actualCapacity] = allocate_storage(newCapacity);

    move_construct_to(newBuffer);
    destroy_elements();
    deallocate_storage(m_buffer, m_capacity);

    m_buffer = newBuffer;
    m_capacity = actualCapacity;
    m_begin = 0;
  }

  template <typename U>
    requires std::constructible_from<T, U &&>
  void push_back(U &&value) {
    if (full()) {
      // Preserve correctness when value aliases an element of *this.
      T temporary{std::forward<U>(value)};
      grow();
      std::construct_at(m_buffer + physical_index(m_size),
                        std::move(temporary));
    } else {
      std::construct_at(m_buffer + physical_index(m_size),
                        std::forward<U>(value));
    }

    ++m_size;
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    if (full()) {
      // Preserve correctness when an argument refers into this buffer.
      T temporary{std::forward<Args>(args)...};
      grow();
      std::construct_at(m_buffer + physical_index(m_size),
                        std::move(temporary));
    } else {
      std::construct_at(m_buffer + physical_index(m_size),
                        std::forward<Args>(args)...);
    }

    const size_type index = physical_index(m_size);
    ++m_size;
    return m_buffer[index];
  }

  template <typename U>
    requires std::constructible_from<T, U &&>
  void push_front(U &&value) {
    if (full()) {
      // Preserve correctness when value aliases an element of *this.
      T temporary{std::forward<U>(value)};
      grow();
      m_begin = decrement(m_begin);
      std::construct_at(m_buffer + m_begin, std::move(temporary));
    } else {
      m_begin = decrement(m_begin);
      std::construct_at(m_buffer + m_begin, std::forward<U>(value));
    }

    ++m_size;
  }

  template <typename... Args> T &emplace_front(Args &&...args) {
    if (full()) {
      // Preserve correctness when an argument refers into this buffer.
      T temporary{std::forward<Args>(args)...};
      grow();
      m_begin = decrement(m_begin);
      std::construct_at(m_buffer + m_begin, std::move(temporary));
    } else {
      m_begin = decrement(m_begin);
      std::construct_at(m_buffer + m_begin, std::forward<Args>(args)...);
    }

    ++m_size;
    return m_buffer[m_begin];
  }

  void pop_back() noexcept {
    assert(!empty());

    const size_type index = physical_index(m_size - 1);
    destroy_at(index);
    --m_size;

    if (empty()) {
      m_begin = 0;
    }
  }

  void pop_front() noexcept {
    assert(!empty());

    destroy_at(m_begin);
    m_begin = increment(m_begin);
    --m_size;

    if (empty()) {
      m_begin = 0;
    }
  }

  T &front() noexcept {
    assert(!empty());
    return m_buffer[m_begin];
  }

  const T &front() const noexcept {
    assert(!empty());
    return m_buffer[m_begin];
  }

  T &back() noexcept {
    assert(!empty());
    return m_buffer[physical_index(m_size - 1)];
  }

  const T &back() const noexcept {
    assert(!empty());
    return m_buffer[physical_index(m_size - 1)];
  }

  T &operator[](size_type index) noexcept {
    assert(index < m_size);
    return m_buffer[physical_index(index)];
  }

  const T &operator[](size_type index) const noexcept {
    assert(index < m_size);
    return m_buffer[physical_index(index)];
  }

  void clear() noexcept {
    destroy_elements();
    m_size = 0;
    m_begin = 0;
  }

private:
  [[nodiscard]] std::pair<T *, size_type> allocate_storage(size_type count) {
    assert(count != 0);

    auto [buffer, capacity] =
        ATraits::template allocate_at_least<T>(m_allocator, count);

    assert(buffer != nullptr);
    assert(capacity >= count);
    return {buffer, capacity};
  }

  void deallocate_storage(T *buffer, size_type capacity) noexcept {
    if (buffer == nullptr) {
      return;
    }

    assert(capacity != 0);
    ATraits::template deallocate<T>(m_allocator, buffer, capacity);
  }

  [[nodiscard]] size_type
  physical_index(size_type logicalIndex) const noexcept {
    assert(m_capacity != 0);
    assert(logicalIndex <= m_size);

    const size_type untilEnd = m_capacity - m_begin;
    return logicalIndex < untilEnd ? m_begin + logicalIndex
                                   : logicalIndex - untilEnd;
  }

  [[nodiscard]] size_type increment(size_type index) const noexcept {
    assert(m_capacity != 0);
    ++index;
    return index == m_capacity ? 0 : index;
  }

  [[nodiscard]] size_type decrement(size_type index) const noexcept {
    assert(m_capacity != 0);
    return index == 0 ? m_capacity - 1 : index - 1;
  }

  void destroy_at(size_type index) noexcept {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(m_buffer + index);
    }
  }

  void destroy_elements() noexcept {
    if (empty()) {
      return;
    }

    if constexpr (!std::is_trivially_destructible_v<T>) {
      const size_type firstCount = std::min(m_size, m_capacity - m_begin);
      std::destroy_n(m_buffer + m_begin, firstCount);

      const size_type secondCount = m_size - firstCount;
      if (secondCount != 0) {
        std::destroy_n(m_buffer, secondCount);
      }
    }
  }

  void copy_construct_from(const VectorDeque &source) {
    assert(m_capacity >= source.m_size);
    assert(m_size == 0);

    if (source.empty()) {
      m_begin = 0;
      return;
    }

    if constexpr (std::is_trivially_copyable_v<T>) {
      const size_type firstCount =
          std::min(source.m_size, source.m_capacity - source.m_begin);
      std::memcpy(m_buffer, source.m_buffer + source.m_begin,
                  firstCount * sizeof(T));

      const size_type secondCount = source.m_size - firstCount;
      if (secondCount != 0) {
        std::memcpy(m_buffer + firstCount, source.m_buffer,
                    secondCount * sizeof(T));
      }
    } else {
      for (size_type i = 0; i < source.m_size; ++i) {
        std::construct_at(m_buffer + i, source[i]);
      }
    }

    m_size = source.m_size;
    m_begin = 0;
  }

  void move_construct_from(VectorDeque &source) {
    assert(m_capacity >= source.m_size);
    assert(m_size == 0);

    if (source.empty()) {
      m_begin = 0;
      return;
    }

    if constexpr (std::is_trivially_copyable_v<T>) {
      const size_type firstCount =
          std::min(source.m_size, source.m_capacity - source.m_begin);
      std::memcpy(m_buffer, source.m_buffer + source.m_begin,
                  firstCount * sizeof(T));

      const size_type secondCount = source.m_size - firstCount;
      if (secondCount != 0) {
        std::memcpy(m_buffer + firstCount, source.m_buffer,
                    secondCount * sizeof(T));
      }
    } else {
      for (size_type i = 0; i < source.m_size; ++i) {
        std::construct_at(m_buffer + i, std::move(source[i]));
      }
    }

    m_size = source.m_size;
    m_begin = 0;
  }

  void move_construct_to(T *destination) {
    if (empty()) {
      return;
    }

    assert(destination != nullptr);

    if constexpr (std::is_trivially_copyable_v<T>) {
      const size_type firstCount = std::min(m_size, m_capacity - m_begin);
      std::memcpy(destination, m_buffer + m_begin, firstCount * sizeof(T));

      const size_type secondCount = m_size - firstCount;
      if (secondCount != 0) {
        std::memcpy(destination + firstCount, m_buffer,
                    secondCount * sizeof(T));
      }
    } else {
      for (size_type i = 0; i < m_size; ++i) {
        std::construct_at(destination + i, std::move((*this)[i]));
      }
    }
  }

  void grow() {
    if (m_capacity == 0) {
      reserve(1);
      return;
    }

    if (m_capacity > std::numeric_limits<size_type>::max() / 2) {
      throw std::length_error("GrowingRingBuffer capacity overflow");
    }

    reserve(m_capacity * 2);
  }

  void reset() noexcept {
    clear();
    deallocate_storage(m_buffer, m_capacity);
    m_buffer = nullptr;
    m_capacity = 0;
  }

  void steal_storage(VectorDeque &other) noexcept {
    m_capacity = std::exchange(other.m_capacity, 0);
    m_size = std::exchange(other.m_size, 0);
    m_begin = std::exchange(other.m_begin, 0);
    m_buffer = std::exchange(other.m_buffer, nullptr);
  }

private:
  size_type m_capacity = 0;
  size_type m_size = 0;
  size_type m_begin = 0;
  T *m_buffer = nullptr;

  [[no_unique_address]]
  A m_allocator;
};

} // namespace strobe
