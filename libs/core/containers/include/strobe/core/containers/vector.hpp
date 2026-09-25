#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <functional>
#include <fmt/format.h>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <strobe/memory.hpp>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Dynamically sized contiguous sequence with allocator support.
 */
template <typename T, Allocator A = strobe::Mallocator> class Vector {
  using ATraits = AllocatorTraits<A>;

public:
  using value_type = T;
  using allocator_type = A;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = T *;
  using const_pointer = const T *;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // =================== Constructors =======================

  /**
   * \brief Constructs, copies, moves, assigns, or destroys a vector.
   * \code{.cpp}
   * explicit Vector(const A& alloc = {});
   * explicit Vector(size_type size, const A& alloc = {});
   * explicit Vector(size_type size, const T& value, const A& alloc = {});
   * template<std::ranges::range R> explicit Vector(const R& range, const A& alloc = {});
   * Vector(const Vector& other);
   * Vector(Vector&& other);
   * Vector& operator=(const Vector& other);
   * Vector& operator=(Vector&& other);
   * ~Vector();
   * \endcode
   * \param alloc Allocator to use.
   * \param size Initial element count.
   * \param value Value used to initialize elements.
   * \param range Source range.
   * \param other Vector to copy or move from.
   * Moved-from vectors are empty.
   */
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
    requires std::ranges::input_range<const Rg> &&
             std::same_as<std::ranges::range_value_t<Rg>, value_type>
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
        copy_construct_range(m_buffer, std::ranges::begin(rg),
                             std::ranges::end(rg));
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
        copy_construct_range(m_buffer, std::ranges::begin(rg),
                             std::ranges::end(rg));
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

  Vector(Vector &&o) noexcept(std::is_nothrow_move_constructible_v<A>)
      : m_capacity(std::exchange(o.m_capacity, 0)),
        m_size(std::exchange(o.m_size, 0)),
        m_buffer(std::exchange(o.m_buffer, nullptr)),
        m_allocator(std::move(o.m_allocator)) {}

  Vector &operator=(Vector &&o) {
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

  /**
   * \brief Accesses an element without bounds checking.
   * \code{.cpp}
   * T& operator[](size_type index) noexcept;
   * const T& operator[](size_type index) const noexcept;
   * \endcode
   * \param index Element index.
   * \return Reference to the element.
   * \attention 1. \p index must be less than size().
   */
  T &operator[](size_type i) {
    assert(i < m_size);
    return m_buffer[i];
  }

  const T &operator[](size_type i) const {
    assert(i < m_size);
    return m_buffer[i];
  }

  /**
   * \brief Appends an element.
   * \code{.cpp}
   * void push_back(const T& value);
   * void push_back(T&& value);
   * \endcode
   * \param value Element to append.
   */
  void push_back(const T &value) {
    assert(m_size < max_size());
    if (m_size == m_capacity) {
      // Preserve correctness when value aliases an element of *this.
      T copy{value};

      grow(recommended_capacity(m_size + 1));

      std::construct_at(m_buffer + m_size, std::move(copy));
    } else {
      std::construct_at(m_buffer + m_size, value);
    }

    ++m_size;
  }

  void push_back(T &&value) {
    assert(m_size < max_size());
    if (m_size == m_capacity) {
      // Preserve correctness when value refers to an element of *this.
      T moved{std::move(value)};

      grow(recommended_capacity(m_size + 1));

      std::construct_at(m_buffer + m_size, std::move(moved));
    } else {
      std::construct_at(m_buffer + m_size, std::move(value));
    }

    ++m_size;
  }

  /**
   * \brief Constructs an element at the back.
   * \code{.cpp}
   * template<typename... Args> T& emplace_back(Args&&... args);
   * \endcode
   * \param args Arguments forwarded to the element constructor.
   * \return Reference to the appended element.
   */
  template <typename... Args> T &emplace_back(Args &&...args) {
    assert(m_size < max_size());
    if (m_size == m_capacity) {
      // Construct first in case args refer into this vector.
      T value(std::forward<Args>(args)...);

      grow(recommended_capacity(m_size + 1));

      std::construct_at(m_buffer + m_size, std::move(value));
    } else {
      std::construct_at(m_buffer + m_size, std::forward<Args>(args)...);
    }

    return m_buffer[m_size++];
  }

  /**
   * \brief Inserts an element at the front.
   * \code{.cpp}
   * void push_front(const T& value);
   * \endcode
   * \param value Element to insert.
   */
  void push_front(const T &value) { insert(begin(), value); }

  /**
   * \brief Removes the last element.
   * \code{.cpp}
   * void pop_back();
   * \endcode
   * \attention 1. The vector must not be empty.
   */
  void pop_back() {
    assert(m_size != 0);

    --m_size;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(m_buffer + m_size);
    }
  }

  /**
   * \brief Removes the first element.
   * \code{.cpp}
   * void pop_front();
   * \endcode
   * \attention 1. The vector must not be empty.
   */
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

  /**
   * \brief Removes all elements.
   * \code{.cpp}
   * void clear() noexcept;
   * \endcode
   * Capacity is retained.
   */
  void clear() noexcept {
    if (m_size == 0) {
      return;
    }

    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_n(m_buffer, m_size);
    }

    m_size = 0;
  }

  /**
   * \brief Returns the number of elements.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   * \return Number of elements.
   */
  [[nodiscard]] size_type size() const noexcept {
    return m_size;
  }

  /**
   * \brief Returns the element capacity.
   * \code{.cpp}
   * size_type capacity() const noexcept;
   * \endcode
   * \return Number of elements that fit without allocation.
   */
  [[nodiscard]] size_type capacity() const noexcept {
    return m_capacity;
  }

  /**
   * \brief Returns the maximum supported size.
   * \code{.cpp}
   * static constexpr size_type max_size() noexcept;
   * \endcode
   * \return Maximum number of elements.
   */
  [[nodiscard]] static constexpr size_type max_size() noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  /**
   * \brief Checks whether the vector is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept {
    return m_size == 0;
  }

  /**
   * \brief Reserves element capacity.
   * \code{.cpp}
   * void reserve(size_type newCapacity);
   * \endcode
   * \param newCapacity Desired capacity.
   * \attention 1. \p newCapacity must not exceed max_size().
   */
  void reserve(size_type newCapacity) {
    assert(newCapacity <= max_size());
    if (newCapacity > m_capacity) {
      grow(newCapacity);
    }
  }

  /**
   * \brief Changes the number of elements.
   * \code{.cpp}
   * void resize(size_type newSize, const T& value);
   * void resize(size_type newSize);
   * \endcode
   * \param newSize New number of elements.
   * \param value Value used to initialize appended elements.
   * \attention 1. The second overload requires T to be default constructible.
   */
  void resize(size_type newSize, const T &value) {
    assert(newSize <= max_size());
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
    assert(newSize <= max_size());
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

  /**
   * \brief Replaces the contents from a range.
   * \code{.cpp}
   * template<std::ranges::sized_range R> void assign(const R& range);
   * template<std::input_iterator It, std::sentinel_for<It> Sent> void assign(It first, Sent last);
   * \endcode
   * \param range Source range.
   * \param first Range beginning.
   * \param last Range end.
   */
  template <std::ranges::sized_range R>
    requires std::ranges::sized_range<const R> &&
             std::same_as<std::ranges::range_value_t<R>, value_type>
  void assign(const R &range) {
    if (references_storage(range)) {
      Vector temporary{range, m_allocator};
      assign(temporary);
      return;
    }

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
        copy_construct_range(m_buffer, std::ranges::begin(range),
                             std::ranges::end(range));
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
        copy_construct_range(m_buffer + m_size, middle,
                             std::ranges::end(range));
      }

      m_size = n;
    }
  }

  template <std::input_iterator It, std::sentinel_for<It> Sent>
    requires std::same_as<std::iter_value_t<It>, value_type>
  void assign(It first, Sent last) {
    // Read the input before modifying *this: first/last may be our iterators.
    Vector temporary{m_allocator};
    if constexpr (std::forward_iterator<It>) {
      temporary.reserve(
          static_cast<size_type>(std::ranges::distance(first, last)));
    }
    for (; first != last; ++first) {
      temporary.push_back(*first);
    }
    *this = std::move(temporary);
  }

  /**
   * \brief Returns the last element.
   * \code{.cpp}
   * T& back();
   * const T& back() const;
   * \endcode
   * \return Reference to the last element.
   * \attention 1. The vector must not be empty.
   */
  T &back() {
    assert(m_size != 0);
    return m_buffer[m_size - 1];
  }

  const T &back() const {
    assert(m_size != 0);
    return m_buffer[m_size - 1];
  }

  /**
   * \brief Returns the first element.
   * \code{.cpp}
   * T& front();
   * const T& front() const;
   * \endcode
   * \return Reference to the first element.
   * \attention 1. The vector must not be empty.
   */
  T &front() {
    assert(m_size != 0);
    return m_buffer[0];
  }

  const T &front() const {
    assert(m_size != 0);
    return m_buffer[0];
  }

  /**
   * \brief Returns element storage.
   * \code{.cpp}
   * T* data() noexcept;
   * const T* data() const noexcept;
   * \endcode
   * \return Pointer to the first element.
   */
  const T *data() const noexcept { return m_buffer; }

  T *data() noexcept { return m_buffer; }

  // ==================== Special Algorithms ========================

  /**
   * \brief Inserts an element.
   * \code{.cpp}
   * iterator insert(const_iterator pos, const T& value);
   * iterator insert(size_type index, const T& value);
   * template<std::ranges::range R> iterator insert(const_iterator pos, const R& range);
   * \endcode
   * \param pos Insertion position.
   * \param index Insertion index.
   * \param value Element to insert.
   * \param range Elements to insert.
   * \return Iterator to the first inserted element.
   */
  iterator insert(const_iterator pos, const T &value) {
    assert(valid_iterator(pos));
    assert(m_size < max_size());

    if (pos == cend()) {
      push_back(value);
      return end() - 1;
    }

    const size_type index = static_cast<size_type>(pos - cbegin());

    // Required in case value refers to an element of this vector.
    T copy{value};

    if (m_size == m_capacity) {
      const size_type newCapacity = recommended_capacity(m_size + 1);

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

  /**
   * \brief Appends elements from a range.
   * \code{.cpp}
   * template<std::ranges::range R> void append(const R& range);
   * \endcode
   * \param range Source range.
   */
  /**
   * \brief Inserts elements from a range.
   * \code{.cpp}
   * template<std::ranges::range R> iterator insert(const_iterator pos, const R& range);
   * \endcode
   * \param pos Insertion position.
   * \param range Elements to insert.
   * \return Iterator to the first inserted element.
   */
  template <std::ranges::range R>
    requires std::ranges::input_range<const R> &&
             std::same_as<std::ranges::range_value_t<R>, value_type>
  void append(const R &range) {
    if (references_storage(range)) {
      Vector temporary{range, m_allocator};
      append(temporary);
      return;
    }

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

      assert(rangeSize <= max_size() - m_size);
      reserve(m_size + rangeSize);

      if constexpr (std::ranges::contiguous_range<R> &&
                    std::is_trivially_copyable_v<T>) {
        std::memmove(m_buffer + m_size, std::ranges::data(range),
                     rangeSize * sizeof(T));
      } else {
        copy_construct_range(m_buffer + m_size, std::ranges::begin(range),
                             std::ranges::end(range));
      }

      m_size += rangeSize;
    }
  }

  template <std::ranges::range R>
    requires std::ranges::input_range<const R> &&
             std::same_as<std::ranges::range_value_t<R>, value_type>
  iterator insert(const_iterator pos, const R &range) {
    assert(valid_iterator(pos));

    if (references_storage(range)) {
      Vector temporary{range, m_allocator};
      return insert(pos, temporary);
    }

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

      assert(n <= max_size() - m_size);
      if (m_size + n > m_capacity) {
        const size_type newCapacity = recommended_capacity(m_size + n);

        auto [newBuffer, actualCapacity] = allocate_storage(newCapacity);

        move_construct_range(newBuffer, m_buffer, index);

        copy_construct_range(newBuffer + index, first, last);

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

          copy_construct_range(m_buffer + m_size, middle, last);

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

  /**
   * \brief Erases an element.
   * \code{.cpp}
   * iterator erase(const_iterator pos);
   * \endcode
   * \param pos Element to erase.
   * \return Iterator following the erased element.
   */
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

  /**
   * \brief Pushes an element onto the back.
   * \code{.cpp}
   * void push(const T& value);
   * \endcode
   * \param value Element to push.
   */
  inline void push(const T &value) { push_back(value); }

  /**
   * \brief Returns the last element.
   * \code{.cpp}
   * T& top();
   * const T& top() const;
   * \endcode
   * \return Reference to the last element.
   * \attention 1. The vector must not be empty.
   */
  inline T &top() { return back(); }

  inline const T &top() const { return back(); }

  /**
   * \brief Removes the last element.
   * \code{.cpp}
   * void pop();
   * \endcode
   * \attention 1. The vector must not be empty.
   */
  inline void pop() { pop_back(); }

  // ================= Set Interface ================

  /**
   * \brief Checks whether a value is present.
   * \code{.cpp}
   * bool contains(const T& value) const;
   * \endcode
   * \param value Value to find.
   * \return Whether the value occurs.
   */
  inline bool contains(const T &value) const {
    const auto e = cend();
    return std::find(cbegin(), e, value) != e;
  }

  /**
   * \brief Adds a value if it is absent.
   * \code{.cpp}
   * bool add(const T& value);
   * \endcode
   * \param value Value to add.
   * \return Whether insertion occurred.
   */
  inline bool add(const T &value) {
    if (contains(value)) {
      return false;
    }

    push_back(value);
    return true;
  }

  /**
   * \brief Removes a value if present.
   * \code{.cpp}
   * bool remove(const T& value);
   * \endcode
   * \param value Value to remove.
   * \return Whether removal occurred.
   * \attention 1. Removing a value does not preserve order.
   */
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

  /**
   * \brief Enqueues an element at the back.
   * \code{.cpp}
   * void enqueue(const T& value);
   * \endcode
   * \param value Element to enqueue.
   */
  void enqueue(const T &value) { push_back(value); }

  /**
   * \brief Returns the first element without removing it.
   * \code{.cpp}
   * const T& peek() const;
   * \endcode
   * \return Reference to the first element.
   * \attention 1. The vector must not be empty.
   */
  const T &peek() const { return front(); }

  /**
   * \brief Removes and returns the first element.
   * \code{.cpp}
   * T dequeue();
   * \endcode
   * \return The removed element.
   * \attention 1. The vector must not be empty.
   */
  T dequeue() {
    assert(m_size != 0);

    T value = std::move(m_buffer[0]);

    pop_front();

    return value;
  }

  // ================= Range Interface ===============

  /**
   * \brief Returns an iterator to the first element.
   * \code{.cpp}
   * iterator begin() noexcept;
   * const_iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first element.
   */
  iterator begin() noexcept { return m_buffer; }

  const_iterator begin() const noexcept { return m_buffer; }

  /**
   * \brief Returns an iterator past the last element.
   * \code{.cpp}
   * iterator end() noexcept;
   * const_iterator end() const noexcept;
   * \endcode
   * \return Iterator past the last element.
   */
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

  /**
   * \brief Returns a reverse iterator to the last element.
   * \code{.cpp}
   * reverse_iterator rbegin() noexcept;
   * const_reverse_iterator rbegin() const noexcept;
   * \endcode
   * \return Reverse iterator to the last element.
   */
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

  /**
   * \brief Returns a reverse iterator before the first element.
   * \code{.cpp}
   * reverse_iterator rend() noexcept;
   * const_reverse_iterator rend() const noexcept;
   * \endcode
   * \return Reverse iterator before the first element.
   */
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /**
   * \brief Returns a const iterator to the first element.
   * \code{.cpp}
   * const_iterator cbegin() const noexcept;
   * \endcode
   * \return Const iterator to the first element.
   */
  const_iterator cbegin() const noexcept { return m_buffer; }

  /**
   * \brief Returns a const iterator past the last element.
   * \code{.cpp}
   * const_iterator cend() const noexcept;
   * \endcode
   * \return Const iterator past the last element.
   */
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

  /**
   * \brief Checks whether an iterator belongs to this vector.
   * \code{.cpp}
   * bool valid_iterator(const_iterator iterator) const noexcept;
   * \endcode
   * \param iterator Iterator to check.
   * \return Whether the iterator is within [begin(), end()].
   */
  bool valid_iterator(const_iterator toCheck) const noexcept {
    if (m_buffer == nullptr) {
      return toCheck == nullptr;
    }

    const std::less<const T *> less;
    return !less(toCheck, cbegin()) && !less(cend(), toCheck);
  }

private:
  [[nodiscard]] size_type recommended_capacity(size_type required) const {
    assert(required > m_capacity && required <= max_size());
    const size_type doubled = m_capacity <= max_size() / 2
                                  ? std::max<size_type>(1, m_capacity * 2)
                                  : max_size();
    return std::max(required, doubled);
  }

  template <std::ranges::range R>
  [[nodiscard]] bool references_storage(const R &range) const {
    if (m_size == 0) {
      return false;
    }
    if constexpr (std::ranges::contiguous_range<const R> &&
                  std::ranges::sized_range<const R>) {
      if (std::ranges::size(range) == 0) {
        return false;
      }
      const T *first = std::ranges::data(range);
      const T *last = first + std::ranges::size(range);
      const std::less<const T *> less;
      return less(first, m_buffer + m_size) && less(m_buffer, last);
    } else {
      // A view or input range may read this vector indirectly.
      return true;
    }
  }

  template <std::input_iterator It, std::sentinel_for<It> Sent>
  static void copy_construct_range(T *dst, It first, Sent last) {
    for (; first != last; ++first, ++dst) {
      std::construct_at(dst, *first);
    }
  }

  [[nodiscard]]
  std::pair<T *, size_type> allocate_storage(size_type count) {
    assert(count != 0);
    assert(count <= max_size());

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

namespace std {

/**
 * \brief Hashes a vector when its element type is hashable.
 * \code{.cpp}
 * template<typename T, strobe::Allocator A>
 * struct hash<strobe::Vector<T, A>>;
 * \endcode
 */
template <typename T, strobe::Allocator A>
  requires requires(const T &value) {
    { std::hash<T>{}(value) } -> std::convertible_to<size_t>;
  }
struct hash<strobe::Vector<T, A>> {
  size_t operator()(const strobe::Vector<T, A> &value) const
      noexcept(noexcept(std::hash<T>{}(*value.begin()))) {
    size_t result = static_cast<size_t>(1469598103934665603ull);
    for (const T &element : value) {
      result ^= std::hash<T>{}(element) +
                static_cast<size_t>(0x9e3779b97f4a7c15ull) +
                (result << 6) + (result >> 2);
    }
    return result;
  }
};

} // namespace std

namespace fmt {

/**
 * \brief Formats a vector as a bracketed list.
 * \code{.cpp}
 * template<typename T, strobe::Allocator A>
 * struct formatter<strobe::Vector<T, A>>;
 * \endcode
 */
template <typename T, strobe::Allocator A>
struct formatter<strobe::Vector<T, A>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::Vector<T, A> &value, FormatContext &ctx) const {
    auto out = ctx.out();
    *out++ = '[';
    for (std::size_t i = 0; i < value.size(); ++i) {
      if (i != 0) {
        *out++ = ',';
        *out++ = ' ';
      }
      out = fmt::format_to(out, "{}", value[i]);
    }
    *out++ = ']';
    return out;
  }
};

} // namespace fmt
