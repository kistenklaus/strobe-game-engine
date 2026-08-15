#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T, std::size_t MinSVOCapacity = 8,
          Allocator A = strobe::Mallocator>
class SmallVector {
private:
  using ATraits = AllocatorTraits<A>;

public:
  using value_type = T;
  using allocator_type = A;
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

  // ================================================================
  // Construction
  // ================================================================

  explicit SmallVector(const A &alloc = {}) : m_allocator(alloc) {
    reset_to_inline();
  }

  explicit SmallVector(size_type size, const A &alloc = {})
    requires std::is_default_constructible_v<T>
      : m_allocator(alloc) {
    reset_to_inline();

    if (size == 0) {
      return;
    }

    reserve_exact_if_required(size);

    std::uninitialized_value_construct_n(m_buffer, size);

    m_size = size;
  }

  SmallVector(size_type size, const T &value, const A &alloc = {})
      : m_allocator(alloc) {
    reset_to_inline();

    if (size == 0) {
      return;
    }

    reserve_exact_if_required(size);

    std::uninitialized_fill_n(m_buffer, size, value);

    m_size = size;
  }

  SmallVector(std::initializer_list<T> values, const A &alloc = {})
      : m_allocator(alloc) {
    reset_to_inline();

    if (values.size() == 0) {
      return;
    }

    reserve_exact_if_required(values.size());

    std::uninitialized_copy(values.begin(), values.end(), m_buffer);

    m_size = values.size();
  }

  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, value_type>
  explicit SmallVector(const R &range, const A &alloc = {})
      : m_allocator(alloc) {
    reset_to_inline();

    if constexpr (std::ranges::sized_range<R>) {
      const size_type n = static_cast<size_type>(std::ranges::size(range));

      if (n == 0) {
        return;
      }

      reserve_exact_if_required(n);

      copy_construct_range(m_buffer, std::ranges::begin(range),
                           std::ranges::end(range));

      m_size = n;
    } else if constexpr (std::ranges::forward_range<R>) {
      const size_type n = static_cast<size_type>(std::ranges::distance(range));

      if (n == 0) {
        return;
      }

      reserve_exact_if_required(n);

      copy_construct_range(m_buffer, std::ranges::begin(range),
                           std::ranges::end(range));

      m_size = n;
    } else {
      for (const T &value : range) {
        push_back(value);
      }
    }
  }

  SmallVector(const SmallVector &other)
      : m_allocator(
            ATraits::select_on_container_copy_construction(other.m_allocator)) {
    reset_to_inline();

    if (other.m_size == 0) {
      return;
    }

    reserve_exact_if_required(other.m_size);

    copy_construct_n(m_buffer, other.m_buffer, other.m_size);

    m_size = other.m_size;
  }

  SmallVector(SmallVector &&other) noexcept(
      std::is_nothrow_move_constructible_v<A> &&
      std::is_nothrow_move_constructible_v<T>)
      : m_allocator(std::move(other.m_allocator)) {
    reset_to_inline();

    if (other.using_inline_storage()) {
      move_construct_n(m_buffer, other.m_buffer, other.m_size);

      m_size = other.m_size;

      destroy_n(other.m_buffer, other.m_size);

      other.m_size = 0;
      return;
    }

    // Heap-backed SmallVector: steal the allocation.
    m_buffer = other.m_buffer;
    m_size = other.m_size;
    m_capacity = other.m_capacity;

    other.reset_to_inline();
  }

  ~SmallVector() {
    destroy_n(m_buffer, m_size);

    if (!using_inline_storage()) {
      deallocate_storage(m_buffer, m_capacity);
    }
  }

  // ================================================================
  // Assignment
  // ================================================================

  SmallVector &operator=(const SmallVector &other) {
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

    assign_copy(other.m_buffer, other.m_size);

    return *this;
  }

  SmallVector &operator=(SmallVector &&other) noexcept(
      std::is_nothrow_move_assignable_v<A> &&
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_assignable_v<T>) {
    if (this == &other) {
      return *this;
    }

    const bool equalAllocator =
        strobe::alloc_equals(m_allocator, other.m_allocator);

    if constexpr (ATraits::propagate_on_container_move_assignment) {
      reset();

      m_allocator = std::move(other.m_allocator);

      move_from_compatible(other);

      return *this;
    }

    if (equalAllocator) {
      reset();
      move_from_compatible(other);
      return *this;
    }

    // Different non-propagating allocators.
    assign_move(other.m_buffer, other.m_size);

    other.clear();

    return *this;
  }

  SmallVector &operator=(std::initializer_list<T> values) {
    assign(values);
    return *this;
  }

  // ================================================================
  // Element access
  // ================================================================

  T &operator[](size_type index) noexcept {
    assert(index < m_size);
    return m_buffer[index];
  }

  const T &operator[](size_type index) const noexcept {
    assert(index < m_size);
    return m_buffer[index];
  }

  T &at(size_type index) {
    if (index >= m_size) {
      throw std::out_of_range{"SmallVector::at"};
    }

    return m_buffer[index];
  }

  const T &at(size_type index) const {
    if (index >= m_size) {
      throw std::out_of_range{"SmallVector::at"};
    }

    return m_buffer[index];
  }

  T &front() noexcept {
    assert(m_size != 0);
    return m_buffer[0];
  }

  const T &front() const noexcept {
    assert(m_size != 0);
    return m_buffer[0];
  }

  T &back() noexcept {
    assert(m_size != 0);
    return m_buffer[m_size - 1];
  }

  const T &back() const noexcept {
    assert(m_size != 0);
    return m_buffer[m_size - 1];
  }

  T *data() noexcept { return m_buffer; }

  const T *data() const noexcept { return m_buffer; }

  // ================================================================
  // Capacity
  // ================================================================

  [[nodiscard]]
  bool empty() const noexcept {
    return m_size == 0;
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
  static constexpr size_type inline_capacity() noexcept {
    return MinSVOCapacity;
  }

  [[nodiscard]]
  bool using_inline_storage() const noexcept {
    if constexpr (MinSVOCapacity == 0) {
      return m_buffer == nullptr;
    } else {
      return m_buffer == inline_data();
    }
  }

  [[nodiscard]]
  constexpr size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  void reserve(size_type newCapacity) {
    if (newCapacity <= m_capacity) {
      return;
    }

    reallocate(newCapacity);
  }

  void shrink_to_fit() {
    if (using_inline_storage()) {
      return;
    }

    if (m_size <= MinSVOCapacity) {
      T *oldBuffer = m_buffer;
      const size_type oldCapacity = m_capacity;

      T *dst = inline_data();

      move_construct_n(dst, oldBuffer, m_size);

      destroy_n(oldBuffer, m_size);

      deallocate_storage(oldBuffer, oldCapacity);

      m_buffer = dst;
      m_capacity = MinSVOCapacity;

      return;
    }

    if (m_size == m_capacity) {
      return;
    }

    reallocate(m_size);
  }

  // ================================================================
  // Modifiers
  // ================================================================

  void clear() noexcept {
    destroy_n(m_buffer, m_size);

    m_size = 0;
  }

  void push_back(const T &value) {
    if (m_size == m_capacity) {
      // Preserve correctness when `value` aliases this vector.
      T copy{value};

      grow_for(m_size + 1);

      std::construct_at(m_buffer + m_size, std::move(copy));
    } else {
      std::construct_at(m_buffer + m_size, value);
    }

    ++m_size;
  }

  void push_back(T &&value) {
    if (m_size == m_capacity) {
      // Preserve correctness if value aliases an existing element.
      T temporary{std::move(value)};

      grow_for(m_size + 1);

      std::construct_at(m_buffer + m_size, std::move(temporary));
    } else {
      std::construct_at(m_buffer + m_size, std::move(value));
    }

    ++m_size;
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    if (m_size == m_capacity) {
      // Args may point into the current buffer.
      T temporary{std::forward<Args>(args)...};

      grow_for(m_size + 1);

      std::construct_at(m_buffer + m_size, std::move(temporary));
    } else {
      std::construct_at(m_buffer + m_size, std::forward<Args>(args)...);
    }

    return m_buffer[m_size++];
  }

  void pop_back() noexcept {
    assert(m_size != 0);

    --m_size;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(m_buffer + m_size);
    }
  }

  void push_front(const T &value) { insert(cbegin(), value); }

  void pop_front() {
    assert(m_size != 0);

    erase(cbegin());
  }

  void resize(size_type newSize)
    requires std::is_default_constructible_v<T>
  {
    if (newSize < m_size) {
      destroy_n(m_buffer + newSize, m_size - newSize);

      m_size = newSize;
      return;
    }

    if (newSize == m_size) {
      return;
    }

    grow_for(newSize);

    std::uninitialized_value_construct(m_buffer + m_size, m_buffer + newSize);

    m_size = newSize;
  }

  void resize(size_type newSize, const T &value) {
    if (newSize < m_size) {
      destroy_n(m_buffer + newSize, m_size - newSize);

      m_size = newSize;
      return;
    }

    if (newSize == m_size) {
      return;
    }

    if (newSize > m_capacity) {
      // Value may alias this vector.
      T copy{value};

      grow_for(newSize);

      std::uninitialized_fill(m_buffer + m_size, m_buffer + newSize, copy);
    } else {
      std::uninitialized_fill(m_buffer + m_size, m_buffer + newSize, value);
    }

    m_size = newSize;
  }

  // ================================================================
  // Assign
  // ================================================================

  void assign(size_type count, const T &value) {
    clear();

    if (count == 0) {
      return;
    }

    reserve(count);

    std::uninitialized_fill_n(m_buffer, count, value);

    m_size = count;
  }

  void assign(std::initializer_list<T> values) {
    assign(values.begin(), values.end());
  }

  template <std::input_iterator It, std::sentinel_for<It> Sent>
    requires std::same_as<std::iter_value_t<It>, value_type>
  void assign(It first, Sent last) {
    SmallVector temporary{m_allocator};

    if constexpr (std::forward_iterator<It>) {
      const size_type n =
          static_cast<size_type>(std::ranges::distance(first, last));

      temporary.reserve(n);
    }

    for (; first != last; ++first) {
      temporary.push_back(*first);
    }

    *this = std::move(temporary);
  }

  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, value_type>
  void assign(const R &range) {
    SmallVector temporary{range, m_allocator};

    *this = std::move(temporary);
  }

  // ================================================================
  // Insert
  // ================================================================

  iterator insert(const_iterator pos, const T &value) {
    assert(valid_iterator(pos));

    const size_type index = index_from_iterator(pos);

    T copy{value};

    insert_one(index, std::move(copy));

    return begin() + index;
  }

  iterator insert(size_type index, const T &value) {
    assert(index <= m_size);

    return insert(iterator_from_index(index), value);
  }

  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, value_type>
  iterator insert(const_iterator pos, const R &range) {
    assert(valid_iterator(pos));

    const size_type index = index_from_iterator(pos);

    // Materializing here makes insertion correct even when the
    // input range aliases this SmallVector.
    SmallVector temporary{range, m_allocator};

    const size_type count = temporary.size();

    if (count == 0) {
      return iterator_from_index(index);
    }

    insert_materialized(index, temporary.data(), count);

    return begin() + index;
  }

  // ================================================================
  // Erase
  // ================================================================

  iterator erase(const_iterator pos) {
    assert(m_size != 0);
    assert(valid_iterator(pos));
    assert(pos != cend());

    const size_type index = index_from_iterator(pos);

    if (index == m_size - 1) {
      pop_back();
      return end();
    }

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memmove(m_buffer + index, m_buffer + index + 1,
                   (m_size - index - 1) * sizeof(T));
    } else {
      std::move(m_buffer + index + 1, m_buffer + m_size, m_buffer + index);
    }

    pop_back();

    return begin() + index;
  }

  iterator erase(const_iterator first, const_iterator last) {
    assert(valid_iterator(first));
    assert(valid_iterator(last));
    assert(first <= last);

    const size_type firstIndex = index_from_iterator(first);

    const size_type lastIndex = index_from_iterator(last);

    const size_type count = lastIndex - firstIndex;

    if (count == 0) {
      return iterator_from_index(firstIndex);
    }

    const size_type tailCount = m_size - lastIndex;

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memmove(m_buffer + firstIndex, m_buffer + lastIndex,
                   tailCount * sizeof(T));
    } else {
      std::move(m_buffer + lastIndex, m_buffer + m_size, m_buffer + firstIndex);
    }

    destroy_n(m_buffer + m_size - count, count);

    m_size -= count;

    return begin() + firstIndex;
  }

  // ================================================================
  // Append
  // ================================================================

  template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, value_type>
  void append(const R &range) {
    // Materialize so ranges into this SmallVector remain valid if
    // growth reallocates.
    SmallVector temporary{range, m_allocator};

    const size_type count = temporary.size();

    if (count == 0) {
      return;
    }

    grow_for(m_size + count);

    copy_construct_n(m_buffer + m_size, temporary.data(), count);

    m_size += count;
  }

  // ================================================================
  // Stack compatibility
  // ================================================================

  void push(const T &value) { push_back(value); }

  T &top() { return back(); }

  const T &top() const { return back(); }

  void pop() { pop_back(); }

  // ================================================================
  // Set-like compatibility
  // ================================================================

  [[nodiscard]]
  bool contains(const T &value) const {
    return std::find(cbegin(), cend(), value) != cend();
  }

  bool add(const T &value) {
    if (contains(value)) {
      return false;
    }

    push_back(value);
    return true;
  }

  // Order is not preserved.
  bool remove(const T &value) {
    iterator it = std::find(begin(), end(), value);

    if (it == end()) {
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

  // ================================================================
  // FIFO compatibility
  // ================================================================

  void enqueue(const T &value) { push_back(value); }

  const T &peek() const { return front(); }

  T dequeue() {
    assert(m_size != 0);

    T result{std::move(front())};

    pop_front();

    return result;
  }

  // ================================================================
  // Iterators
  // ================================================================

  iterator begin() noexcept { return m_buffer; }

  const_iterator begin() const noexcept { return m_buffer; }

  const_iterator cbegin() const noexcept { return m_buffer; }

  iterator end() noexcept { return iterator_from_index(m_size); }

  const_iterator end() const noexcept { return iterator_from_index(m_size); }

  const_iterator cend() const noexcept { return iterator_from_index(m_size); }

  reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }

  reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{cend()};
  }

  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{cbegin()};
  }

  [[nodiscard]]
  A get_allocator() const {
    return m_allocator;
  }

private:
  // ================================================================
  // Inline storage
  // ================================================================

  T *inline_data() noexcept {
    if constexpr (MinSVOCapacity == 0) {
      return nullptr;
    } else {
      return reinterpret_cast<T *>(m_inlineStorage.data());
    }
  }

  const T *inline_data() const noexcept {
    if constexpr (MinSVOCapacity == 0) {
      return nullptr;
    } else {
      return reinterpret_cast<const T *>(m_inlineStorage.data());
    }
  }

  void reset_to_inline() noexcept {
    m_buffer = inline_data();
    m_size = 0;
    m_capacity = MinSVOCapacity;
  }

  // ================================================================
  // Allocation
  // ================================================================

  [[nodiscard]]
  T *allocate_storage(size_type capacity) {
    assert(capacity != 0);

    T *result = ATraits::template allocate<T>(m_allocator, capacity);

    assert(result != nullptr);

    return result;
  }

  void deallocate_storage(T *ptr, size_type capacity) noexcept {
    if (ptr == nullptr) {
      return;
    }

    assert(capacity != 0);

    ATraits::template deallocate<T>(m_allocator, ptr, capacity);
  }

  void reserve_exact_if_required(size_type required) {
    if (required <= m_capacity) {
      return;
    }

    reallocate(required);
  }

  [[nodiscard]]
  size_type recommended_capacity(size_type required) const {
    assert(required > m_capacity);

    if (m_capacity == 0) {
      return std::max<size_type>(required, 1);
    }

    const size_type max = max_size();

    if (m_capacity > max / 2) {
      return required;
    }

    return std::max(required, m_capacity * 2);
  }

  void grow_for(size_type required) {
    if (required <= m_capacity) {
      return;
    }

    reallocate(recommended_capacity(required));
  }

  void reallocate(size_type newCapacity) {
    assert(newCapacity >= m_size);
    assert(newCapacity > m_capacity);

    T *newBuffer = allocate_storage(newCapacity);

    move_construct_n(newBuffer, m_buffer, m_size);

    destroy_n(m_buffer, m_size);

    if (!using_inline_storage()) {
      deallocate_storage(m_buffer, m_capacity);
    }

    m_buffer = newBuffer;
    m_capacity = newCapacity;
  }

  void reset() noexcept {
    destroy_n(m_buffer, m_size);

    if (!using_inline_storage()) {
      deallocate_storage(m_buffer, m_capacity);
    }

    reset_to_inline();
  }

  // ================================================================
  // Construction/destruction helpers
  // ================================================================

  static void destroy_n(T *ptr, size_type count) noexcept {
    if (count == 0) {
      return;
    }

    assert(ptr != nullptr);

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_n(ptr, count);
    }
  }

  static void copy_construct_n(T *dst, const T *src, size_type count) {
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

  template <std::input_iterator It, std::sentinel_for<It> Sent>
  static void copy_construct_range(T *dst, It first, Sent last) {
    std::uninitialized_copy(first, last, dst);
  }

  static void move_construct_n(T *dst, T *src, size_type count) {
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

  // ================================================================
  // Assignment helpers
  // ================================================================

  void assign_copy(const T *src, size_type count) {
    if (count > m_capacity) {
      reset();

      reserve_exact_if_required(count);

      copy_construct_n(m_buffer, src, count);

      m_size = count;
      return;
    }

    const size_type common = std::min(m_size, count);

    if constexpr (std::is_trivially_copyable_v<T>) {
      if (count != 0) {
        std::memmove(m_buffer, src, count * sizeof(T));
      }

      m_size = count;
      return;
    }

    std::copy_n(src, common, m_buffer);

    if (count > m_size) {
      std::uninitialized_copy(src + m_size, src + count, m_buffer + m_size);
    } else if (count < m_size) {
      destroy_n(m_buffer + count, m_size - count);
    }

    m_size = count;
  }

  void assign_move(T *src, size_type count) {
    if (count > m_capacity) {
      reset();

      reserve_exact_if_required(count);

      move_construct_n(m_buffer, src, count);

      m_size = count;
      return;
    }

    const size_type common = std::min(m_size, count);

    if constexpr (std::is_trivially_copyable_v<T>) {
      if (count != 0) {
        std::memmove(m_buffer, src, count * sizeof(T));
      }

      m_size = count;
      return;
    }

    std::move(src, src + common, m_buffer);

    if (count > m_size) {
      std::uninitialized_move(src + m_size, src + count, m_buffer + m_size);
    } else if (count < m_size) {
      destroy_n(m_buffer + count, m_size - count);
    }

    m_size = count;
  }

  void move_from_compatible(SmallVector &other) {
    if (!other.using_inline_storage()) {
      m_buffer = other.m_buffer;
      m_size = other.m_size;
      m_capacity = other.m_capacity;

      other.reset_to_inline();

      return;
    }

    reserve_exact_if_required(other.m_size);

    move_construct_n(m_buffer, other.m_buffer, other.m_size);

    m_size = other.m_size;

    destroy_n(other.m_buffer, other.m_size);

    other.m_size = 0;
  }

  // ================================================================
  // Insert helpers
  // ================================================================

  void insert_one(size_type index, T &&value) {
    assert(index <= m_size);

    if (index == m_size) {
      push_back(std::move(value));

      return;
    }

    grow_for(m_size + 1);

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memmove(m_buffer + index + 1, m_buffer + index,
                   (m_size - index) * sizeof(T));

      std::memcpy(m_buffer + index, &value, sizeof(T));
    } else {
      std::construct_at(m_buffer + m_size, std::move(m_buffer[m_size - 1]));

      std::move_backward(m_buffer + index, m_buffer + m_size - 1,
                         m_buffer + m_size);

      m_buffer[index] = std::move(value);
    }

    ++m_size;
  }

  void insert_materialized(size_type index, const T *values, size_type count) {
    assert(index <= m_size);

    if (count == 0) {
      return;
    }

    grow_for(m_size + count);

    const size_type tail = m_size - index;

    if constexpr (std::is_trivially_copyable_v<T>) {
      std::memmove(m_buffer + index + count, m_buffer + index,
                   tail * sizeof(T));

      std::memcpy(m_buffer + index, values, count * sizeof(T));

      m_size += count;
      return;
    }

    if (count <= tail) {
      std::uninitialized_move(m_buffer + m_size - count, m_buffer + m_size,
                              m_buffer + m_size);

      std::move_backward(m_buffer + index, m_buffer + m_size - count,
                         m_buffer + m_size);

      std::copy_n(values, count, m_buffer + index);
    } else {
      const size_type extra = count - tail;

      std::uninitialized_copy(values + tail, values + count, m_buffer + m_size);

      std::uninitialized_move(m_buffer + index, m_buffer + m_size,
                              m_buffer + index + count);

      std::copy_n(values, tail, m_buffer + index);

      (void)extra;
    }

    m_size += count;
  }

  // ================================================================
  // Iterator helpers
  // ================================================================

  [[nodiscard]]
  size_type index_from_iterator(const_iterator it) const noexcept {
    if (m_size == 0) {
      assert(it == m_buffer);
      return 0;
    }

    return static_cast<size_type>(it - m_buffer);
  }

  [[nodiscard]]
  iterator iterator_from_index(size_type index) noexcept {
    assert(index <= m_size);

    if (m_buffer == nullptr) {
      assert(index == 0);
      return nullptr;
    }

    return m_buffer + index;
  }

  [[nodiscard]]
  const_iterator iterator_from_index(size_type index) const noexcept {
    assert(index <= m_size);

    if (m_buffer == nullptr) {
      assert(index == 0);
      return nullptr;
    }

    return m_buffer + index;
  }

  [[nodiscard]]
  bool valid_iterator(const_iterator it) const noexcept {
    if (m_buffer == nullptr) {
      return it == nullptr;
    }

    return it >= m_buffer && it <= m_buffer + m_size;
  }

private:
  /*
   * MinSVOCapacity is the number of T elements stored inline.
   *
   * std::array<byte, 0> keeps SmallVector<T, 0> valid as well.
   */
  alignas(
      T) std::array<std::byte, sizeof(T) * MinSVOCapacity> m_inlineStorage{};

  T *m_buffer = nullptr;
  size_type m_size = 0;
  size_type m_capacity = 0;

  [[no_unique_address]]
  A m_allocator;
};

} // namespace strobe
