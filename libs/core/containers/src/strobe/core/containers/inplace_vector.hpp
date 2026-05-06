#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T, std::size_t Capacity> class InplaceVector {
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using reference = T &;
  using const_reference = const T &;

  using pointer = T *;
  using const_pointer = const T *;

  using iterator = T *;
  using const_iterator = const T *;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  static constexpr size_type STORAGE_CAPACITY = Capacity == 0 ? 1 : Capacity;

  struct Slot {
    alignas(T) std::byte storage[sizeof(T)];
  };

  [[nodiscard]] pointer ptr(size_type index) noexcept {
    return std::launder(reinterpret_cast<pointer>(m_storage[index].storage));
  }

  [[nodiscard]] const_pointer ptr(size_type index) const noexcept {
    return std::launder(
        reinterpret_cast<const_pointer>(m_storage[index].storage));
  }

  void destroy_range(size_type first, size_type last) noexcept {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_type i = first; i < last; ++i) {
        std::destroy_at(ptr(i));
      }
    }
  }

private:
  Slot m_storage[STORAGE_CAPACITY];
  size_type m_size = 0;

public:
  constexpr InplaceVector() noexcept = default;

  InplaceVector(const InplaceVector &other)
    requires std::copy_constructible<T>
  {
    size_type constructed = 0;
    const size_type other_size = other.size();

    try {
      for (; constructed < other_size; ++constructed) {
        std::construct_at(ptr(constructed), other[constructed]);
      }
    } catch (...) {
      destroy_range(0, constructed);
      throw;
    }

    m_size = other_size;
  }

  InplaceVector(InplaceVector &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires std::move_constructible<T>
  {
    size_type constructed = 0;
    const size_type other_size = other.size();

    try {
      for (; constructed < other_size; ++constructed) {
        std::construct_at(ptr(constructed), std::move(other[constructed]));
      }
    } catch (...) {
      destroy_range(0, constructed);
      throw;
    }

    m_size = other_size;
    other.clear();
  }

  InplaceVector &operator=(const InplaceVector &other)
    requires std::copy_constructible<T> && std::is_copy_assignable_v<T>
  {
    if (this == &other) {
      return *this;
    }

    const size_type other_size = other.size();

    if (other_size <= m_size) {
      for (size_type i = 0; i < other_size; ++i) {
        (*this)[i] = other[i];
      }

      destroy_range(other_size, m_size);
      m_size = other_size;
      return *this;
    }

    size_type i = 0;

    for (; i < m_size; ++i) {
      (*this)[i] = other[i];
    }

    try {
      for (; i < other_size; ++i) {
        std::construct_at(ptr(i), other[i]);
      }
    } catch (...) {
      destroy_range(m_size, i);
      throw;
    }

    m_size = other_size;
    return *this;
  }

  InplaceVector &operator=(InplaceVector &&other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_assignable_v<T>)
    requires std::move_constructible<T> && std::is_move_assignable_v<T>
  {
    if (this == &other) {
      return *this;
    }

    const size_type other_size = other.size();

    if (other_size <= m_size) {
      for (size_type i = 0; i < other_size; ++i) {
        (*this)[i] = std::move(other[i]);
      }

      destroy_range(other_size, m_size);
      m_size = other_size;
      other.clear();
      return *this;
    }

    size_type i = 0;

    for (; i < m_size; ++i) {
      (*this)[i] = std::move(other[i]);
    }

    try {
      for (; i < other_size; ++i) {
        std::construct_at(ptr(i), std::move(other[i]));
      }
    } catch (...) {
      destroy_range(m_size, i);
      throw;
    }

    m_size = other_size;
    other.clear();
    return *this;
  }

  ~InplaceVector() noexcept { clear(); }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

  [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }

  [[nodiscard]] static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  [[nodiscard]] static constexpr size_type max_size() noexcept {
    return Capacity;
  }

  [[nodiscard]] pointer data() noexcept { return ptr(0); }

  [[nodiscard]] const_pointer data() const noexcept { return ptr(0); }

  [[nodiscard]] iterator begin() noexcept { return data(); }
  [[nodiscard]] const_iterator begin() const noexcept { return data(); }
  [[nodiscard]] const_iterator cbegin() const noexcept { return data(); }

  [[nodiscard]] iterator end() noexcept { return data() + m_size; }
  [[nodiscard]] const_iterator end() const noexcept { return data() + m_size; }
  [[nodiscard]] const_iterator cend() const noexcept { return data() + m_size; }

  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return reverse_iterator{end()};
  }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{cend()};
  }

  [[nodiscard]] reverse_iterator rend() noexcept {
    return reverse_iterator{begin()};
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  [[nodiscard]] const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{cbegin()};
  }

  [[nodiscard]] std::span<T> span() noexcept { return {data(), m_size}; }

  [[nodiscard]] std::span<const T> span() const noexcept {
    return {data(), m_size};
  }

  [[nodiscard]] reference operator[](size_type index) noexcept {
    assert(index < m_size);
    return data()[index];
  }

  [[nodiscard]] const_reference operator[](size_type index) const noexcept {
    assert(index < m_size);
    return data()[index];
  }

  [[nodiscard]] reference at(size_type index) {
    if (index >= m_size) {
      throw std::out_of_range{"InplaceVector::at"};
    }

    return data()[index];
  }

  [[nodiscard]] const_reference at(size_type index) const {
    if (index >= m_size) {
      throw std::out_of_range{"InplaceVector::at"};
    }

    return data()[index];
  }

  [[nodiscard]] reference front() noexcept {
    assert(m_size != 0);
    return data()[0];
  }

  [[nodiscard]] const_reference front() const noexcept {
    assert(m_size != 0);
    return data()[0];
  }

  [[nodiscard]] reference back() noexcept {
    assert(m_size != 0);
    return data()[m_size - 1];
  }

  [[nodiscard]] const_reference back() const noexcept {
    assert(m_size != 0);
    return data()[m_size - 1];
  }

  template <typename... Args> reference emplace_back(Args &&...args) {
    assert(m_size < Capacity);

    std::construct_at(ptr(m_size), std::forward<Args>(args)...);
    ++m_size;

    return back();
  }

  void push_back(const T &value) { emplace_back(value); }

  void push_back(T &&value) { emplace_back(std::move(value)); }

  void pop_back() noexcept {
    assert(m_size != 0);

    --m_size;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(ptr(m_size));
    }
  }

  void clear() noexcept {
    destroy_range(0, m_size);
    m_size = 0;
  }

  void resize(size_type new_size)
    requires std::default_initializable<T>
  {
    assert(new_size <= Capacity);

    if (new_size < m_size) {
      destroy_range(new_size, m_size);
      m_size = new_size;
      return;
    }

    while (m_size < new_size) {
      emplace_back();
    }
  }

  void resize(size_type new_size, const T &value) {
    assert(new_size <= Capacity);

    if (new_size < m_size) {
      destroy_range(new_size, m_size);
      m_size = new_size;
      return;
    }

    while (m_size < new_size) {
      emplace_back(value);
    }
  }
};

} // namespace strobe
