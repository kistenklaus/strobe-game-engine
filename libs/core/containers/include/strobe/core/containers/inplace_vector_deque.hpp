#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T, size_t Capacity> class InplaceVectorDeque {
public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T &;
  using const_reference = const T &;

private:
  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  static constexpr size_type storage_capacity = Capacity == 0 ? 1 : Capacity;

  [[nodiscard]] T *ptr(size_type index) noexcept {
    return reinterpret_cast<T *>(m_storage[index].storage);
  }

  [[nodiscard]] const T *ptr(size_type index) const noexcept {
    return reinterpret_cast<const T *>(m_storage[index].storage);
  }

  [[nodiscard]] size_type physical_index(size_type index) const noexcept {
    const size_type remaining = Capacity - m_begin;
    return index < remaining ? m_begin + index : index - remaining;
  }

  [[nodiscard]] size_type increment(size_type index) const noexcept {
    return index + 1 == Capacity ? 0 : index + 1;
  }

  [[nodiscard]] size_type decrement(size_type index) const noexcept {
    return index == 0 ? Capacity - 1 : index - 1;
  }

public:
  InplaceVectorDeque() noexcept = default;

  InplaceVectorDeque(const InplaceVectorDeque &other)
    requires std::is_copy_constructible_v<T>
  {
    for (size_type i = 0; i < other.m_size; ++i) {
      emplace_back(other[i]);
    }
  }

  InplaceVectorDeque(InplaceVectorDeque &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_constructible_v<T>
  {
    for (size_type i = 0; i < other.m_size; ++i) {
      emplace_back(std::move(other[i]));
    }
    other.clear();
  }

  InplaceVectorDeque &operator=(const InplaceVectorDeque &other)
    requires std::is_copy_constructible_v<T>
  {
    if (this != &other) {
      clear();
      for (size_type i = 0; i < other.m_size; ++i) {
        emplace_back(other[i]);
      }
    }
    return *this;
  }

  InplaceVectorDeque &operator=(InplaceVectorDeque &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires std::is_move_constructible_v<T>
  {
    if (this != &other) {
      clear();
      for (size_type i = 0; i < other.m_size; ++i) {
        emplace_back(std::move(other[i]));
      }
      other.clear();
    }
    return *this;
  }

  ~InplaceVectorDeque() noexcept { clear(); }

  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
  [[nodiscard]] bool full() const noexcept { return m_size == Capacity; }
  [[nodiscard]] size_type size() const noexcept { return m_size; }
  [[nodiscard]] static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    assert(!full() && "InplaceVectorDeque is full");

    const size_type index = physical_index(m_size);
    T *value = std::construct_at(ptr(index), std::forward<Args>(args)...);
    ++m_size;
    return *value;
  }

  template <typename... Args> T &emplace_front(Args &&...args) {
    assert(!full() && "InplaceVectorDeque is full");

    const size_type index = decrement(m_begin);
    T *value = std::construct_at(ptr(index), std::forward<Args>(args)...);
    m_begin = index;
    ++m_size;
    return *value;
  }

  void push_back(const T &value) { emplace_back(value); }
  void push_back(T &&value) { emplace_back(std::move(value)); }

  void push_front(const T &value) { emplace_front(value); }
  void push_front(T &&value) { emplace_front(std::move(value)); }

  void pop_back() noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");

    std::destroy_at(ptr(physical_index(m_size - 1)));
    --m_size;
    if (empty()) {
      m_begin = 0;
    }
  }

  void pop_front() noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");

    std::destroy_at(ptr(m_begin));
    m_begin = increment(m_begin);
    --m_size;
    if (empty()) {
      m_begin = 0;
    }
  }

  [[nodiscard]] T &front() noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");
    return *std::launder(ptr(m_begin));
  }

  [[nodiscard]] const T &front() const noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");
    return *std::launder(ptr(m_begin));
  }

  [[nodiscard]] T &back() noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");
    return (*this)[m_size - 1];
  }

  [[nodiscard]] const T &back() const noexcept {
    assert(!empty() && "InplaceVectorDeque is empty");
    return (*this)[m_size - 1];
  }

  [[nodiscard]] T &operator[](size_type index) noexcept {
    assert(index < m_size);
    return *std::launder(ptr(physical_index(index)));
  }

  [[nodiscard]] const T &operator[](size_type index) const noexcept {
    assert(index < m_size);
    return *std::launder(ptr(physical_index(index)));
  }

  void clear() noexcept {
    for (size_type i = 0; i < m_size; ++i) {
      std::destroy_at(ptr(physical_index(i)));
    }
    m_size = 0;
    m_begin = 0;
  }

private:
  Slot m_storage[storage_capacity];
  size_type m_size = 0;
  size_type m_begin = 0;
};

} // namespace strobe
