#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>

namespace strobe::flow {

namespace detail {
template <typename> struct ArgumentSpanAccess;
}

/**
 * \brief Provides indirect typed access to a fixed group of runtime resources.
 * \code{.cpp}
 * template<typename T>
 * class ArgumentSpan;
 * \endcode
 *
 * Resources in an argument span are independently stored. Each element is a
 * pointer and may be null when the corresponding recorded resource was null.
 */
template <typename T> class ArgumentSpan {
  static_assert(std::is_object_v<T>);

public:
  /**
   * \brief Iterates over indirectly stored runtime resources.
   * \code{.cpp}
   * class iterator;
   * \endcode
   */
  class iterator {
  public:
    using value_type = std::remove_cv_t<T>;
    using difference_type = std::ptrdiff_t;
    using reference = T *;
    using iterator_category = std::forward_iterator_tag;

    reference operator*() const noexcept { return m_span->get(m_index); }
    iterator &operator++() noexcept {
      ++m_index;
      return *this;
    }
    iterator operator++(int) noexcept {
      auto result = *this;
      ++*this;
      return result;
    }
    iterator &operator--() noexcept {
      --m_index;
      return *this;
    }
    iterator &operator+=(difference_type offset) noexcept {
      m_index += offset;
      return *this;
    }
    iterator &operator-=(difference_type offset) noexcept {
      m_index -= offset;
      return *this;
    }
    reference operator[](difference_type offset) const noexcept {
      return m_span->get(m_index + offset);
    }
    friend iterator operator+(iterator value, difference_type offset) noexcept {
      value += offset;
      return value;
    }
    friend iterator operator-(iterator value, difference_type offset) noexcept {
      value -= offset;
      return value;
    }
    friend difference_type operator-(iterator left, iterator right) noexcept {
      return static_cast<difference_type>(left.m_index) -
             static_cast<difference_type>(right.m_index);
    }
    friend bool operator==(iterator, iterator) noexcept = default;
    friend auto operator<=>(iterator, iterator) noexcept = default;

  private:
    friend class ArgumentSpan;
    explicit iterator(const ArgumentSpan *span, std::size_t index) noexcept
        : m_span(span), m_index(index) {}
    const ArgumentSpan *m_span = nullptr;
    std::size_t m_index = 0;
  };

  /**
   * \brief Constructs an empty argument span.
   * \code{.cpp}
   * ArgumentSpan() noexcept;
   * \endcode
   */
  ArgumentSpan() noexcept = default;

  /**
   * \brief Returns the number of grouped runtime resources.
   * \code{.cpp}
   * std::size_t size() const noexcept;
   * \endcode
   *
   * \return Number of resources in the compiled argument group.
   */
  std::size_t size() const noexcept { return m_size; }

  /**
   * \brief Tests whether the argument span is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   *
   * \return Whether the span contains no resources.
   */
  bool empty() const noexcept { return m_size == 0; }

  /**
   * \brief Returns one grouped runtime resource.
   * \code{.cpp}
   * T* operator[](std::size_t index) const noexcept;
   * \endcode
   *
   * \param index Zero-based resource index.
   * \return Pointer to the runtime resource, or null for an absent resource.
   * \attention 1 `index` must be less than `size()`.
   */
  T *operator[](std::size_t index) const noexcept {
    assert(index < m_size);
    return get(index);
  }

  /**
   * \brief Returns iterators over the grouped runtime resources.
   * \code{.cpp}
   * iterator begin() const noexcept;
   * iterator end() const noexcept;
   * \endcode
   *
   * \return Iterator at the corresponding span boundary.
   */
  iterator begin() const noexcept { return iterator(this, 0); }
  iterator end() const noexcept { return iterator(this, m_size); }

private:
  template <typename> friend struct detail::ArgumentSpanAccess;

  ArgumentSpan(void *const *pointers, const std::uint32_t *elements,
               std::size_t size) noexcept
      : m_pointers(pointers), m_elements(elements), m_size(size) {}

  T *get(std::size_t index) const noexcept {
    const auto element = m_elements[index];
    return element == std::numeric_limits<std::uint32_t>::max()
               ? nullptr
               : static_cast<T *>(m_pointers[element]);
  }

  void *const *m_pointers = nullptr;
  const std::uint32_t *m_elements = nullptr;
  std::size_t m_size = 0;
};

namespace detail {
template <typename T> struct ArgumentSpanAccess {
  static ArgumentSpan<T> make(void *const *pointers,
                              const std::uint32_t *elements,
                              std::size_t size) noexcept {
    return ArgumentSpan<T>(pointers, elements, size);
  }
};
} // namespace detail

} // namespace strobe::flow
