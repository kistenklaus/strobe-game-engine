#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T>
class VectorStorage {
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = T*;
  using const_iterator = const T*;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
  constexpr VectorStorage() noexcept = default;

  VectorStorage(const VectorStorage&) = delete;
  VectorStorage& operator=(const VectorStorage&) = delete;

  constexpr VectorStorage(VectorStorage&& other) noexcept
      : m_data(other.m_data),
        m_size(other.m_size),
        m_capacity(other.m_capacity) {
    other.m_data = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
  }

  VectorStorage& operator=(VectorStorage&&) = delete;

  ~VectorStorage() {
    assert(m_data == nullptr &&
           "VectorStorage must be destroyed with destroy(alloc) first");
  }

  [[nodiscard]] constexpr bool empty() const noexcept {
    return m_size == 0;
  }

  [[nodiscard]] constexpr size_type size() const noexcept {
    return m_size;
  }

  [[nodiscard]] constexpr size_type capacity() const noexcept {
    return m_capacity;
  }

  [[nodiscard]] constexpr size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  [[nodiscard]] constexpr pointer data() noexcept {
    return m_data;
  }

  [[nodiscard]] constexpr const_pointer data() const noexcept {
    return m_data;
  }

  [[nodiscard]] constexpr iterator begin() noexcept {
    return m_data;
  }

  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return m_data;
  }

  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return m_data;
  }

  [[nodiscard]] constexpr iterator end() noexcept {
    return m_data == nullptr ? nullptr : m_data + m_size;
  }

  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return m_data == nullptr ? nullptr : m_data + m_size;
  }

  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return m_data == nullptr ? nullptr : m_data + m_size;
  }

  [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator{end()};
  }

  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{cend()};
  }

  [[nodiscard]] constexpr reverse_iterator rend() noexcept {
    return reverse_iterator{begin()};
  }

  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{cbegin()};
  }

  [[nodiscard]] constexpr std::span<T> span() noexcept {
    return {m_data, m_size};
  }

  [[nodiscard]] constexpr std::span<const T> span() const noexcept {
    return {m_data, m_size};
  }

  [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
    assert(index < m_size);
    return m_data[index];
  }

  [[nodiscard]] constexpr const_reference operator[](
      size_type index) const noexcept {
    assert(index < m_size);
    return m_data[index];
  }

  [[nodiscard]] reference at(size_type index) {
    if (index >= m_size) {
      throw std::out_of_range{"VectorStorage::at"};
    }

    return m_data[index];
  }

  [[nodiscard]] const_reference at(size_type index) const {
    if (index >= m_size) {
      throw std::out_of_range{"VectorStorage::at"};
    }

    return m_data[index];
  }

  [[nodiscard]] constexpr reference front() noexcept {
    assert(m_size != 0);
    return m_data[0];
  }

  [[nodiscard]] constexpr const_reference front() const noexcept {
    assert(m_size != 0);
    return m_data[0];
  }

  [[nodiscard]] constexpr reference back() noexcept {
    assert(m_size != 0);
    return m_data[m_size - 1];
  }

  [[nodiscard]] constexpr const_reference back() const noexcept {
    assert(m_size != 0);
    return m_data[m_size - 1];
  }

  template <Allocator Alloc>
  void reserve(Alloc& alloc, size_type new_capacity) {
    if (new_capacity <= m_capacity) {
      return;
    }

    reallocate(alloc, new_capacity);
  }

  template <Allocator Alloc>
  void shrink_to_fit(Alloc& alloc) {
    if (m_size == m_capacity) {
      return;
    }

    if (m_size == 0) {
      destroy(alloc);
      return;
    }

    reallocate(alloc, m_size);
  }

  template <Allocator Alloc, typename... Args>
  reference emplace_back(Alloc& alloc, Args&&... args) {
    if (m_size == m_capacity) {
      reserve(alloc, growth_capacity(m_size + 1));
    }

    std::construct_at(m_data + m_size, std::forward<Args>(args)...);
    ++m_size;

    return m_data[m_size - 1];
  }

  template <Allocator Alloc>
  void push_back(Alloc& alloc, const T& value) {
    emplace_back(alloc, value);
  }

  template <Allocator Alloc>
  void push_back(Alloc& alloc, T&& value) {
    emplace_back(alloc, std::move(value));
  }

  template <Allocator Alloc>
  void pop_back(Alloc&) noexcept {
    assert(m_size != 0);

    --m_size;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(m_data + m_size);
    }
  }

  template <Allocator Alloc>
  void clear(Alloc&) noexcept {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy(m_data, m_data + m_size);
    }

    m_size = 0;
  }

  template <Allocator Alloc>
  void resize(Alloc& alloc, size_type new_size)
    requires std::default_initializable<T>
  {
    if (new_size < m_size) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy(m_data + new_size, m_data + m_size);
      }

      m_size = new_size;
      return;
    }

    if (new_size > m_size) {
      reserve(alloc, new_size);

      size_type constructed = m_size;

      try {
        for (; constructed < new_size; ++constructed) {
          std::construct_at(m_data + constructed);
        }
      } catch (...) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
          std::destroy(m_data + m_size, m_data + constructed);
        }

        throw;
      }

      m_size = new_size;
    }
  }

  template <Allocator Alloc>
  void resize(Alloc& alloc, size_type new_size, const T& value) {
    if (new_size < m_size) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy(m_data + new_size, m_data + m_size);
      }

      m_size = new_size;
      return;
    }

    if (new_size > m_size) {
      reserve(alloc, new_size);

      size_type constructed = m_size;

      try {
        for (; constructed < new_size; ++constructed) {
          std::construct_at(m_data + constructed, value);
        }
      } catch (...) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
          std::destroy(m_data + m_size, m_data + constructed);
        }

        throw;
      }

      m_size = new_size;
    }
  }

  template <Allocator Alloc>
  iterator erase(Alloc& alloc, const_iterator pos) {
    assert(pos >= begin());
    assert(pos < end());

    const size_type index = static_cast<size_type>(pos - begin());
    return erase(alloc, begin() + index, begin() + index + 1);
  }

  template <Allocator Alloc>
  iterator erase(Alloc&, const_iterator first, const_iterator last) {
    assert(first >= begin());
    assert(first <= last);
    assert(last <= end());

    if (first == last) {
      return const_cast<iterator>(first);
    }

    static_assert(std::is_move_assignable_v<T>,
                  "erase requires T to be move-assignable");

    const size_type first_index = static_cast<size_type>(first - begin());
    const size_type last_index = static_cast<size_type>(last - begin());
    const size_type count = last_index - first_index;

    iterator dst = m_data + first_index;
    iterator src = m_data + last_index;

    for (; src != end(); ++src, ++dst) {
      *dst = std::move(*src);
    }

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy(m_data + (m_size - count), m_data + m_size);
    }

    m_size -= count;

    return m_data + first_index;
  }

  template <Allocator Alloc, typename... Args>
  iterator emplace(Alloc& alloc, const_iterator pos, Args&&... args) {
    assert(pos >= begin());
    assert(pos <= end());

    static_assert(std::is_move_constructible_v<T>,
                  "emplace requires T to be move-constructible");
    static_assert(std::is_move_assignable_v<T>,
                  "emplace requires T to be move-assignable");

    const size_type index = static_cast<size_type>(pos - begin());

    if (index == m_size) {
      emplace_back(alloc, std::forward<Args>(args)...);
      return m_data + index;
    }

    if (m_size == m_capacity) {
      reserve(alloc, growth_capacity(m_size + 1));
    }

    std::construct_at(m_data + m_size, std::move(m_data[m_size - 1]));

    for (size_type i = m_size - 1; i > index; --i) {
      m_data[i] = std::move(m_data[i - 1]);
    }

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(m_data + index);
    }

    std::construct_at(m_data + index, std::forward<Args>(args)...);

    ++m_size;
    return m_data + index;
  }

  template <Allocator Alloc>
  iterator insert(Alloc& alloc, const_iterator pos, const T& value) {
    return emplace(alloc, pos, value);
  }

  template <Allocator Alloc>
  iterator insert(Alloc& alloc, const_iterator pos, T&& value) {
    return emplace(alloc, pos, std::move(value));
  }

  template <Allocator Alloc>
  void assign(Alloc& alloc, size_type count, const T& value) {
    clear(alloc);
    reserve(alloc, count);

    size_type constructed = 0;

    try {
      for (; constructed < count; ++constructed) {
        std::construct_at(m_data + constructed, value);
      }
    } catch (...) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy(m_data, m_data + constructed);
      }

      throw;
    }

    m_size = count;
  }

  template <Allocator Alloc, typename InputIt>
  void assign(Alloc& alloc, InputIt first, InputIt last) {
    clear(alloc);

    if constexpr (std::forward_iterator<InputIt>) {
      const auto count = static_cast<size_type>(std::distance(first, last));
      reserve(alloc, count);
    }

    for (; first != last; ++first) {
      emplace_back(alloc, *first);
    }
  }

  template <Allocator Alloc>
  void assign(Alloc& alloc, std::initializer_list<T> init) {
    assign(alloc, init.begin(), init.end());
  }

  template <Allocator Alloc>
  void destroy(Alloc& alloc) noexcept {
    clear(alloc);

    if (m_data != nullptr) {
      AllocatorTraits<Alloc>::template deallocate<T>(
          alloc, m_data, m_capacity);
    }

    m_data = nullptr;
    m_size = 0;
    m_capacity = 0;
  }

  template <Allocator Alloc>
  void move_assign_from(Alloc& alloc, VectorStorage&& other) noexcept {
    if (this == &other) {
      return;
    }

    destroy(alloc);

    m_data = other.m_data;
    m_size = other.m_size;
    m_capacity = other.m_capacity;

    other.m_data = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
  }

private:
  [[nodiscard]] size_type growth_capacity(size_type min_capacity) const {
    size_type new_capacity =
        m_capacity == 0 ? 8 : m_capacity + m_capacity / 2 + 1;

    if (new_capacity < min_capacity) {
      new_capacity = min_capacity;
    }

    return new_capacity;
  }

  template <Allocator Alloc>
  void reallocate(Alloc& alloc, size_type new_capacity) {
    assert(new_capacity >= m_size);

    pointer new_data =
        AllocatorTraits<Alloc>::template allocate<T>(alloc, new_capacity);

    size_type constructed = 0;

    try {
      for (; constructed < m_size; ++constructed) {
        if constexpr (std::is_nothrow_move_constructible_v<T> ||
                      !std::is_copy_constructible_v<T>) {
          std::construct_at(
              new_data + constructed,
              std::move(m_data[constructed]));
        } else {
          std::construct_at(new_data + constructed, m_data[constructed]);
        }
      }
    } catch (...) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        std::destroy(new_data, new_data + constructed);
      }

      if (new_data != nullptr) {
        AllocatorTraits<Alloc>::template deallocate<T>(
            alloc, new_data, new_capacity);
      }

      throw;
    }

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy(m_data, m_data + m_size);
    }

    if (m_data != nullptr) {
      AllocatorTraits<Alloc>::template deallocate<T>(
          alloc, m_data, m_capacity);
    }

    m_data = new_data;
    m_capacity = new_capacity;
  }

private:
  pointer m_data = nullptr;
  size_type m_size = 0;
  size_type m_capacity = 0;
};

} // namespace strobe
