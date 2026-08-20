#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <ranges>
#include <strobe/memory.hpp>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T, Allocator A = strobe::Mallocator> class Vector {
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
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // =================== Constructors =======================

  explicit Vector(const A &alloc = {}) : m_allocator(alloc) {}

  explicit Vector(size_type size, const A &alloc = {})
    requires std::is_default_constructible_v<T>
      : m_size(size), m_allocator(alloc) {
    if (size == 0) {
      return;
    }

    auto [buffer, capacity] = allocate_storage(size);
    m_buffer = buffer;
    m_capacity = capacity;
    std::uninitialized_value_construct_n(m_buffer, size);
  }

  explicit Vector(size_type size, const T &value, const A &alloc = {})
      : m_size(size), m_allocator(alloc) {
    if (size == 0) {
      return;
    }

    auto [buffer, capacity] = allocate_storage(size);
    m_buffer = buffer;
    m_capacity = capacity;
    std::uninitialized_fill_n(m_buffer, size, value);
  }

  template <std::ranges::range Rg>
    requires std::same_as<std::ranges::range_value_t<Rg>, value_type>
  explicit Vector(const Rg &rg, const A &alloc = {}) : m_allocator(alloc) {
    if constexpr (std::ranges::sized_range<Rg>) {
      const size_type n = static_cast<size_type>(std::ranges::size(rg));

      if (n == 0) {
        return;
      }

      auto [buffer, capacity] = allocate_storage(n);
      m_buffer = buffer;
      m_capacity = capacity;
      m_size = n;

      if constexpr (std::ranges::contiguous_range<Rg> &&
                    std::is_trivially_copyable_v<T>) {
        std::memcpy(m_buffer, std::ranges::data(rg), n * sizeof(T));
      } else {
        std::uninitialized_copy(std::ranges::begin(rg), std::ranges::end(rg),
                                m_buffer);
      }
    } else if constexpr (std::ranges::forward_range<Rg>) {
      const size_type n = static_cast<size_type>(std::ranges::distance(rg));

      if (n == 0) {
        return;
      }

      auto [buffer, capacity] = allocate_storage(n);
      m_buffer = buffer;
      m_capacity = capacity;
      m_size = n;

      if constexpr (std::ranges::contiguous_range<Rg> &&
                    std::is_trivially_copyable_v<T>) {
        std::memcpy(m_buffer, std::ranges::data(rg), n * sizeof(T));
      } else {
        std::uninitialized_copy(std::ranges::begin(rg), std::ranges::end(rg),
                                m_buffer);
      }
    } else {
      for (const T &value : rg) {
        push_back(value);
      }
    }
  }

  ~Vector() { reset(); }

  Vector(const Vector &o)
      : m_size(o.m_size),
        m_allocator(
            ATraits::select_on_container_copy_construction(o.m_allocator)) {
    if (m_size == 0) {
      return;
    }

    auto [buffer, capacity] = allocate_storage(m_size);
    m_buffer = buffer;
    m_capacity = capacity;
    copy_construct_from(o.m_buffer, m_size);
  }

  Vector &operator=(const Vector &o) {
    if (this == &o) {
      return *this;
    }

    const bool equalAllocator =
        strobe::alloc_equals(m_allocator, o.m_allocator);

    if constexpr (ATraits::propagate_on_container_copy_assignment) {
      if (!equalAllocator) {
        reset();
      }

      m_allocator = o.m_allocator;
    }

    if (o.m_size > m_capacity) {
      reset();

      if (o.m_size != 0) {
        auto [buffer, capacity] = allocate_storage(o.m_size);
        m_buffer = buffer;
        m_capacity = capacity;
      }

      copy_construct_from(o.m_buffer, o.m_size);
    } else {
      copy_assign_from(o.m_buffer, o.m_size);
    }

    return *this;
  }

  Vector(Vector &&o) noexcept
      : m_capacity(std::exchange(o.m_capacity, 0)),
        m_size(std::exchange(o.m_size, 0)),
        m_buffer(std::exchange(o.m_buffer, nullptr)),
        m_allocator(std::move(o.m_allocator)) {}

  Vector &operator=(Vector &&o) noexcept {
    if (this == &o) {
      return *this;
    }

    const bool equalAllocator =
        strobe::alloc_equals(m_allocator, o.m_allocator);

    if constexpr (ATraits::propagate_on_container_move_assignment) {
      reset();

      m_allocator = std::move(o.m_allocator);
      steal_storage(o);

      return *this;
    }

    if (equalAllocator) {
      reset();
      steal_storage(o);

      return *this;
    }

    // Allocators are incompatible. Move the elements instead.
    if (o.m_size > m_capacity) {
      reset();

      if (o.m_size != 0) {
        auto [buffer, capacity] = allocate_storage(o.m_size);
        m_buffer = buffer;
        m_capacity = capacity;
      }

      move_construct_from(o.m_buffer, o.m_size);
    } else {
      move_assign_from(o.m_buffer, o.m_size);
    }

    o.reset();

    return *this;
  }

  // ===================== Vector Interface ===================

  T &operator[](size_type i) {
    assert(i < m_size);
    return m_buffer[i];
  }

  const T &operator[](size_type i) const {
    assert(i < m_size);
    return m_buffer[i];
  }

  void push_back(const T &value) {
    if (m_size == m_capacity) {
      // Preserve correctness when value aliases an element of *this.
      T copy{value};

      grow(m_capacity == 0 ? 2 : m_capacity * 2);

      std::construct_at(m_buffer + m_size, std::move(copy));
    } else {
      std::construct_at(m_buffer + m_size, value);
    }

    ++m_size;
  }

  void push_back(T &&value) {
    if (m_size == m_capacity) {
      // Preserve correctness when value refers to an element of *this.
      T moved{std::move(value)};

      grow(m_capacity == 0 ? 1 : m_capacity * 2);

      std::construct_at(m_buffer + m_size, std::move(moved));
    } else {
      std::construct_at(m_buffer + m_size, std::move(value));
    }

    ++m_size;
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    if (m_size == m_capacity) {
      // Construct first in case args refer into this vector.
      T value{std::forward<Args>(args)...};

      grow(m_capacity == 0 ? 1 : m_capacity * 2);

      std::construct_at(m_buffer + m_size, std::move(value));
    } else {
      std::construct_at(m_buffer + m_size, std::forward<Args>(args)...);
    }

    return m_buffer[m_size++];
  }

  void push_front(const T &value) { insert(begin(), value); }

  void pop_back() {
    assert(m_size != 0);

    --m_size;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(m_buffer + m_size);
    }
  }

  void pop_front() {
    assert(m_size != 0);

    if constexpr (std::is_trivially_copyable_v<T>) {
      if (m_size > 1) {
        std::memmove(m_buffer, m_buffer + 1, (m_size - 1) * sizeof(T));
      }
    } else {
      std::move(m_buffer + 1, m_buffer + m_size, m_buffer);

      std::destroy_at(m_buffer + m_size - 1);
    }

    --m_size;
  }

  void clear() noexcept {
    if (m_size == 0) {
      return;
    }

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_n(m_buffer, m_size);
    }

    m_size = 0;
  }

  [[nodiscard]]
  size_type size() const noexcept {
    return m_size;
  }

  [[nodiscard]]
  size_type capacity() const noexcept {
    return m_capacity;
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return m_size == 0;
  }

  void reserve(size_type newCapacity) {
    if (newCapacity > m_capacity) {
      grow(newCapacity);
    }
  }

  void resize(size_type newSize, const T &value) {
    if (newSize < m_size) {
      destroy_range(m_buffer + newSize, m_size - newSize);
    } else if (newSize > m_size) {
      if (newSize > m_capacity) {
        // value may point into the current buffer.
        T copy{value};

        grow(newSize);

        std::uninitialized_fill(m_buffer + m_size, m_buffer + newSize, copy);
      } else {
        std::uninitialized_fill(m_buffer + m_size, m_buffer + newSize, value);
      }
    }

    m_size = newSize;
  }

  void resize(size_type newSize)
    requires std::is_default_constructible_v<T>
  {
    if (newSize < m_size) {
      destroy_range(m_buffer + newSize, m_size - newSize);
    } else if (newSize > m_size) {
      if (newSize > m_capacity) {
        grow(newSize);
      }

      std::uninitialized_value_construct(m_buffer + m_size, m_buffer + newSize);
    }

    m_size = newSize;
  }

  template <std::ranges::sized_range R>
    requires std::same_as<std::ranges::range_value_t<R>, value_type>
  void assign(const R &range) {
    const size_type n = static_cast<size_type>(std::ranges::size(range));

    if (n == 0) {
      clear();
      return;
    }

    if (n > m_capacity) {
      reset();

      auto [buffer, capacity] = allocate_storage(n);
      m_buffer = buffer;
      m_capacity = capacity;

      if constexpr (std::ranges::contiguous_range<R> &&
                    std::is_trivially_copyable_v<T>) {
        std::memcpy(m_buffer, std::ranges::data(range), n * sizeof(T));
      } else {
        std::uninitialized_copy(std::ranges::begin(range),
                                std::ranges::end(range), m_buffer);
      }

      m_size = n;
      return;
    }

    auto first = std::ranges::begin(range);

    if constexpr (std::ranges::contiguous_range<R> &&
                  std::is_trivially_copyable_v<T>) {
      std::memmove(m_buffer, std::ranges::data(range), n * sizeof(T));

      m_size = n;
    } else {
      const size_type assigned = std::min(m_size, n);

      auto middle = first;
      std::ranges::advance(middle, assigned);

      std::copy(first, middle, m_buffer);

      if (n < m_size) {
        destroy_range(m_buffer + n, m_size - n);
      } else if (n > m_size) {
        std::uninitialized_copy(middle, std::ranges::end(range),
                                m_buffer + m_size);
      }

      m_size = n;
    }
  }

  template <std::input_iterator It, std::sentinel_for<It> Sent>
    requires std::same_as<std::iter_value_t<It>, value_type>
  void assign(It first, Sent last) {
    clear();

    if constexpr (std::forward_iterator<It>) {
      const size_type n =
          static_cast<size_type>(std::ranges::distance(first, last));

      if (n == 0) {
        return;
      }

      reserve(n);

      if constexpr (std::contiguous_iterator<It> && std::same_as<It, Sent> &&
                    std::is_trivially_copyable_v<T>) {
        std::memcpy(m_buffer, std::to_address(first), n * sizeof(T));
      } else {
        std::uninitialized_copy(first, last, m_buffer);
      }

      m_size = n;
    } else {
      for (; first != last; ++first) {
        push_back(*first);
      }
    }
  }

  T &back() {
    assert(m_size != 0);
    return m_buffer[m_size - 1];
  }

  const T &back() const {
    assert(m_size != 0);
    return m_buffer[m_size - 1];
  }

  T &front() {
    assert(m_size != 0);
    return m_buffer[0];
  }

  const T &front() const {
    assert(m_size != 0);
    return m_buffer[0];
  }

  const T *data() const noexcept { return m_buffer; }

  T *data() noexcept { return m_buffer; }

  // ==================== Special Algorithms ========================

  iterator insert(const_iterator pos, const T &value) {
    assert(valid_iterator(pos));

    if (pos == cend()) {
      push_back(value);
      return end() - 1;
    }

    const size_type index = static_cast<size_type>(pos - cbegin());

    // Required in case value refers to an element of this vector.
    T copy{value};

    if (m_size == m_capacity) {
      const size_type newCapacity = m_capacity == 0 ? 1 : m_capacity * 2;

      assert(newCapacity > m_capacity);

      auto [newBuffer, actualCapacity] = allocate_storage(newCapacity);

      move_construct_range(newBuffer, m_buffer, index);

      std::construct_at(newBuffer + index, std::move(copy));

      move_construct_range(newBuffer + index + 1, m_buffer + index,
                           m_size - index);

      destroy_range(m_buffer, m_size);
      deallocate_storage(m_buffer, m_capacity);

      m_buffer = newBuffer;
      m_capacity = actualCapacity;
    } else {
      if constexpr (std::is_trivially_copyable_v<T>) {
        std::memmove(m_buffer + index + 1, m_buffer + index,
                     (m_size - index) * sizeof(T));

        std::memcpy(m_buffer + index, &copy, sizeof(T));
      } else {
        // Construct the final element into the uninitialized slot.
        std::construct_at(m_buffer + m_size, std::move(m_buffer[m_size - 1]));

        // Shift the remaining initialized elements.
        std::move_backward(m_buffer + index, m_buffer + m_size - 1,
                           m_buffer + m_size);

        m_buffer[index] = std::move(copy);
      }
    }

    ++m_size;
    return begin() + index;
  }

  iterator insert(size_type i, const T &value) {
    assert(i <= m_size);

    const_iterator pos = i == 0 ? cbegin() : cbegin() + i;

    return insert(pos, value);
  }

  // ========================= Range insertion ==================

  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, value_type>
  void append(const R &range) {
    if constexpr (!std::ranges::forward_range<R> &&
                  !std::ranges::sized_range<R>) {
      for (const T &value : range) {
        push_back(value);
      }

      return;
    } else {
      const size_type rangeSize = static_cast<size_type>([&]() {
        if constexpr (std::ranges::sized_range<R>) {
          return std::ranges::size(range);
        } else {
          return std::ranges::distance(range);
        }
      }());

      if (rangeSize == 0) {
        return;
      }

      reserve(m_size + rangeSize);

      if constexpr (std::ranges::contiguous_range<R> &&
                    std::is_trivially_copyable_v<T>) {
        std::memmove(m_buffer + m_size, std::ranges::data(range),
                     rangeSize * sizeof(T));
      } else {
        std::uninitialized_copy(std::ranges::begin(range),
                                std::ranges::end(range), m_buffer + m_size);
      }

      m_size += rangeSize;
    }
  }

  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, value_type>
  iterator insert(const_iterator pos, const R &range) {
    assert(valid_iterator(pos));

    // Materialize single-pass ranges before modifying this vector.
    if constexpr (!std::ranges::forward_range<R>) {
      Vector temporary{range, m_allocator};
      return insert(pos, temporary);
    } else {
      const size_type n = static_cast<size_type>([&]() {
        if constexpr (std::ranges::sized_range<R>) {
          return std::ranges::size(range);
        } else {
          return std::ranges::distance(range);
        }
      }());

      if (n == 0) {
        return const_cast<iterator>(pos);
      }

      const size_type index =
          m_buffer == nullptr ? 0 : static_cast<size_type>(pos - cbegin());

      if (index == m_size) {
        append(range);
        return begin() + index;
      }

      auto first = std::ranges::begin(range);

      auto last = std::ranges::end(range);

      if (m_size + n > m_capacity) {
        const size_type newCapacity = std::max(
            m_capacity != 0 ? m_capacity * 2 : size_type{1}, m_size + n);

        auto [newBuffer, actualCapacity] = allocate_storage(newCapacity);

        move_construct_range(newBuffer, m_buffer, index);

        std::uninitialized_copy(first, last, newBuffer + index);

        move_construct_range(newBuffer + index + n, m_buffer + index,
                             m_size - index);

        destroy_range(m_buffer, m_size);

        deallocate_storage(m_buffer, m_capacity);

        m_buffer = newBuffer;
        m_capacity = actualCapacity;
      } else {
        const size_type tail = m_size - index;

        if constexpr (std::ranges::contiguous_range<R> &&
                      std::is_trivially_copyable_v<T>) {
          std::memmove(m_buffer + index + n, m_buffer + index,
                       tail * sizeof(T));

          std::memmove(m_buffer + index, std::ranges::data(range),
                       n * sizeof(T));
        } else if (n <= tail) {
          // Move the last n initialized elements into raw storage.
          std::uninitialized_move(m_buffer + m_size - n, m_buffer + m_size,
                                  m_buffer + m_size);

          // Shift the remaining initialized range right.
          std::move_backward(m_buffer + index, m_buffer + m_size - n,
                             m_buffer + m_size);

          auto src = first;

          for (size_type i = 0; i < n; ++i, ++src) {
            m_buffer[index + i] = *src;
          }
        } else {
          // n > tail:
          //
          // [ existing tail ]
          //
          // becomes
          //
          // [ inserted prefix ][ inserted suffix ][ moved tail ]
          //
          auto middle = first;
          std::ranges::advance(middle, tail);

          std::uninitialized_copy(middle, last, m_buffer + m_size);

          std::uninitialized_move(m_buffer + index, m_buffer + m_size,
                                  m_buffer + index + n);

          auto src = first;

          for (size_type i = 0; i < tail; ++i, ++src) {
            m_buffer[index + i] = *src;
          }
        }
      }

      m_size += n;

      return begin() + index;
    }
  }

  iterator erase(const_iterator pos) {
    assert(m_size != 0);
    assert(valid_iterator(pos));
    assert(pos != cend());

    const size_type index = static_cast<size_type>(pos - cbegin());

    if (index == m_size - 1) {
      pop_back();
      return end();
    }

    if constexpr (std::is_trivially_copyable_v<T>) {
      const size_type n = m_size - 1 - index;

      std::memmove(m_buffer + index, m_buffer + index + 1, sizeof(T) * n);
    } else {
      std::move(m_buffer + index + 1, m_buffer + m_size, m_buffer + index);
    }

    pop_back();

    return begin() + index;
  }

  // ================= Stack Interface ==============

  inline void push(const T &value) { push_back(value); }

  inline T &top() { return back(); }

  inline const T &top() const { return back(); }

  inline void pop() { pop_back(); }

  // ================= Set Interface ================

  inline bool contains(const T &value) const {
    const auto e = cend();
    return std::find(cbegin(), e, value) != e;
  }

  inline bool add(const T &value) {
    if (contains(value)) {
      return false;
    }

    push_back(value);
    return true;
  }

  // NOTE: The order of elements is undefined after removing an element.
  inline bool remove(const T &value) {
    const auto e = end();
    auto it = std::find(begin(), e, value);

    if (it == e) {
      return false;
    }

    const size_type index = static_cast<size_type>(it - begin());

    const size_type lastIndex = m_size - 1;

    if (index != lastIndex) {
      m_buffer[index] = std::move(m_buffer[lastIndex]);
    }

    pop_back();

    return true;
  }

  // ================= FIFO Queue Interface ================

  void enqueue(const T &value) { push_back(value); }

  const T &peek() const { return front(); }

  T dequeue() {
    assert(m_size != 0);

    T value = std::move(m_buffer[0]);

    pop_front();

    return value;
  }

  // ================= Range Interface ===============

  iterator begin() noexcept { return m_buffer; }

  const_iterator begin() const noexcept { return m_buffer; }

  iterator end() noexcept {
    if (m_buffer == nullptr) {
      return nullptr;
    }

    return m_buffer + m_size;
  }

  const_iterator end() const noexcept {
    if (m_buffer == nullptr) {
      return nullptr;
    }

    return m_buffer + m_size;
  }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  const_iterator cbegin() const noexcept { return m_buffer; }

  const_iterator cend() const noexcept {
    if (m_buffer == nullptr) {
      return nullptr;
    }

    return m_buffer + m_size;
  }

  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

  bool _validIterator(const_iterator toCheck) const noexcept {
    if (m_buffer == nullptr) {
      return toCheck == nullptr;
    }

    return cbegin() <= toCheck && toCheck <= cend();
  }

private:
  [[nodiscard]]
  std::pair<T *, size_type> allocate_storage(size_type count) {
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

  static void destroy_range(T *buffer, size_type count) noexcept {
    if (count == 0) {
      return;
    }

    assert(buffer != nullptr);

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_n(buffer, count);
    }
  }

  static void copy_construct_range(T *dst, const T *src, size_type count) {
    if (count == 0) {
      return;
    }

    assert(dst != nullptr);
    assert(src != nullptr);

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memcpy(dst, src, count * sizeof(T));
    } else {
      std::uninitialized_copy_n(src, count, dst);
    }
  }

  static void move_construct_range(T *dst, T *src, size_type count) {
    if (count == 0) {
      return;
    }

    assert(dst != nullptr);
    assert(src != nullptr);

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memcpy(dst, src, count * sizeof(T));
    } else {
      std::uninitialized_move_n(src, count, dst);
    }
  }

  void grow(size_type newCapacity) {
    ZoneScopedN("Vector::grow");
    assert(newCapacity > m_capacity);
    assert(newCapacity != 0);

    T *oldBuffer = m_buffer;
    const size_type oldCapacity = m_capacity;

    auto [newBuffer, actualCapacity] = allocate_storage(newCapacity);

    move_construct_range(newBuffer, oldBuffer, m_size);

    destroy_range(oldBuffer, m_size);

    deallocate_storage(oldBuffer, oldCapacity);

    m_buffer = newBuffer;
    m_capacity = actualCapacity;
  }

  void reset() noexcept {
    destroy_range(m_buffer, m_size);

    release();
  }

  void release() noexcept {
    deallocate_storage(m_buffer, m_capacity);

    m_buffer = nullptr;
    m_capacity = 0;
    m_size = 0;
  }

  void copy_construct_from(const T *source, size_type size) {
    assert(m_capacity >= size);

    if (size == 0) {
      m_size = 0;
      return;
    }

    assert(m_buffer != nullptr);
    assert(source != nullptr);

    copy_construct_range(m_buffer, source, size);

    m_size = size;
  }

  void copy_assign_from(const T *source, size_type size) {
    assert(m_capacity >= size);

    if (size == 0) {
      clear();
      return;
    }

    assert(m_buffer != nullptr);
    assert(source != nullptr);

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memmove(m_buffer, source, size * sizeof(T));

      m_size = size;
      return;
    }

    const size_type common = std::min(m_size, size);

    std::copy_n(source, common, m_buffer);

    if (size < m_size) {
      destroy_range(m_buffer + size, m_size - size);
    } else if (size > m_size) {
      std::uninitialized_copy(source + m_size, source + size,
                              m_buffer + m_size);
    }

    m_size = size;
  }

  void move_construct_from(T *source, size_type size) {
    assert(m_capacity >= size);

    if (size == 0) {
      m_size = 0;
      return;
    }

    assert(m_buffer != nullptr);
    assert(source != nullptr);

    move_construct_range(m_buffer, source, size);

    m_size = size;
  }

  void move_assign_from(T *source, size_type size) {
    assert(m_capacity >= size);

    if (size == 0) {
      clear();
      return;
    }

    assert(m_buffer != nullptr);
    assert(source != nullptr);

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memmove(m_buffer, source, size * sizeof(T));

      m_size = size;
      return;
    }

    const size_type common = std::min(m_size, size);

    std::move(source, source + common, m_buffer);

    if (size < m_size) {
      destroy_range(m_buffer + size, m_size - size);
    } else if (size > m_size) {
      std::uninitialized_move(source + m_size, source + size,
                              m_buffer + m_size);
    }

    m_size = size;
  }

  void steal_storage(Vector &o) noexcept {
    m_buffer = std::exchange(o.m_buffer, nullptr);

    m_capacity = std::exchange(o.m_capacity, 0);

    m_size = std::exchange(o.m_size, 0);
  }

private:
  size_type m_capacity = 0;
  size_type m_size = 0;
  T *m_buffer = nullptr;

  [[no_unique_address]]
  A m_allocator;
};

} // namespace strobe
