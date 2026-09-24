#pragma once

#include "strobe/core/containers/boxed_array.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T, Allocator A = Mallocator> class VectorDeque {
public:
  using value_type = T;
  using allocator_type = std::remove_cvref_t<A>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T &;
  using const_reference = const T &;

private:
  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  using storage_type = BoxedArray<Slot, allocator_type>;

public:
  explicit VectorDeque(const allocator_type &alloc = {})
      : m_slots(0, alloc), m_allocator(alloc) {}

  explicit VectorDeque(size_type capacity, const allocator_type &alloc = {})
      : m_slots(capacity, alloc), m_allocator(alloc) {}

  VectorDeque(const VectorDeque &other)
    requires std::copy_constructible<T>
      : m_slots(other.capacity(), other.m_allocator),
        m_allocator(other.m_allocator) {
    for (size_type i = 0; i < other.m_size; ++i) {
      std::construct_at(slot(i), other[i]);
      ++m_size;
    }
  }

  VectorDeque(VectorDeque &&other) noexcept(
      std::is_nothrow_move_constructible_v<storage_type> &&
      std::is_nothrow_move_constructible_v<allocator_type>)
      : m_slots(std::move(other.m_slots)),
        m_allocator(std::move(other.m_allocator)),
        m_size(std::exchange(other.m_size, 0)),
        m_begin(std::exchange(other.m_begin, 0)) {}

  VectorDeque &operator=(const VectorDeque &other)
    requires std::copy_constructible<T>
  {
    if (this != &other) {
      VectorDeque copy(other);
      *this = std::move(copy);
    }
    return *this;
  }

  VectorDeque &operator=(VectorDeque &&other) noexcept(
      std::is_nothrow_move_assignable_v<storage_type> &&
      std::is_nothrow_move_assignable_v<allocator_type>) {
    if (this != &other) {
      clear();
      m_slots = std::move(other.m_slots);
      m_allocator = std::move(other.m_allocator);
      m_size = std::exchange(other.m_size, 0);
      m_begin = std::exchange(other.m_begin, 0);
    }
    return *this;
  }

  ~VectorDeque() noexcept { clear(); }

  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
  [[nodiscard]] bool full() const noexcept { return m_size == capacity(); }
  [[nodiscard]] size_type size() const noexcept { return m_size; }
  [[nodiscard]] size_type capacity() const noexcept { return m_slots.size(); }

  void reserve(size_type new_capacity) {
    if (new_capacity <= capacity()) {
      return;
    }

    storage_type replacement(new_capacity, m_allocator);
    for (size_type i = 0; i < m_size; ++i) {
      std::construct_at(pointer_at(replacement, i), std::move((*this)[i]));
    }

    const size_type old_size = m_size;
    clear();
    m_slots = std::move(replacement);
    m_size = old_size;
  }

  template <typename U>
    requires std::constructible_from<T, U &&>
  void push_back(U &&value) {
    emplace_back(std::forward<U>(value));
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    if (full()) {
      T temporary(std::forward<Args>(args)...);
      grow();
      std::construct_at(slot(m_size), std::move(temporary));
    } else {
      std::construct_at(slot(m_size), std::forward<Args>(args)...);
    }

    ++m_size;
    return back();
  }

  template <typename U>
    requires std::constructible_from<T, U &&>
  void push_front(U &&value) {
    emplace_front(std::forward<U>(value));
  }

  template <typename... Args> T &emplace_front(Args &&...args) {
    if (full()) {
      T temporary(std::forward<Args>(args)...);
      grow();
      m_begin = decrement(m_begin);
      std::construct_at(pointer_at(m_slots, m_begin), std::move(temporary));
    } else {
      if (capacity() == 0) {
        grow();
      }
      m_begin = decrement(m_begin);
      std::construct_at(pointer_at(m_slots, m_begin),
                        std::forward<Args>(args)...);
    }

    ++m_size;
    return front();
  }

  void pop_back() noexcept {
    assert(!empty());
    std::destroy_at(slot(m_size - 1));
    --m_size;
    if (empty()) {
      m_begin = 0;
    }
  }

  void pop_front() noexcept {
    assert(!empty());
    std::destroy_at(pointer_at(m_slots, m_begin));
    m_begin = increment(m_begin);
    --m_size;
    if (empty()) {
      m_begin = 0;
    }
  }

  [[nodiscard]] T &front() noexcept {
    assert(!empty());
    return *std::launder(pointer_at(m_slots, m_begin));
  }

  [[nodiscard]] const T &front() const noexcept {
    assert(!empty());
    return *std::launder(pointer_at(m_slots, m_begin));
  }

  [[nodiscard]] T &back() noexcept {
    assert(!empty());
    return (*this)[m_size - 1];
  }

  [[nodiscard]] const T &back() const noexcept {
    assert(!empty());
    return (*this)[m_size - 1];
  }

  [[nodiscard]] T &operator[](size_type index) noexcept {
    assert(index < m_size);
    return *std::launder(slot(index));
  }

  [[nodiscard]] const T &operator[](size_type index) const noexcept {
    assert(index < m_size);
    return *std::launder(slot(index));
  }

  void clear() noexcept {
    for (size_type i = 0; i < m_size; ++i) {
      std::destroy_at(slot(i));
    }
    m_size = 0;
    m_begin = 0;
  }

private:
  [[nodiscard]] static T *pointer_at(storage_type &storage,
                                     size_type index) noexcept {
    return reinterpret_cast<T *>(storage[index].storage);
  }

  [[nodiscard]] static const T *pointer_at(const storage_type &storage,
                                           size_type index) noexcept {
    return reinterpret_cast<const T *>(storage[index].storage);
  }

  [[nodiscard]] size_type physical_index(size_type index) const noexcept {
    const size_type remaining = capacity() - m_begin;
    return index < remaining ? m_begin + index : index - remaining;
  }

  [[nodiscard]] T *slot(size_type index) noexcept {
    return pointer_at(m_slots, physical_index(index));
  }

  [[nodiscard]] const T *slot(size_type index) const noexcept {
    return pointer_at(m_slots, physical_index(index));
  }

  [[nodiscard]] size_type increment(size_type index) const noexcept {
    return index + 1 == capacity() ? 0 : index + 1;
  }

  [[nodiscard]] size_type decrement(size_type index) const noexcept {
    return index == 0 ? capacity() - 1 : index - 1;
  }

  void grow() {
    assert(capacity() <= std::numeric_limits<size_type>::max() / 2);
    reserve(capacity() == 0 ? 1 : capacity() * 2);
  }

  storage_type m_slots;
  [[no_unique_address]] allocator_type m_allocator;
  size_type m_size = 0;
  size_type m_begin = 0;
};

} // namespace strobe
