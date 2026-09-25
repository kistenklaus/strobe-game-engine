#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <algorithm>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <fmt/format.h>
#include <limits>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Heap-allocated array with inline metadata.
 * \code{.cpp}
 * template<typename T, Allocator Alloc = Mallocator>
 * class BoxedArray;
 * \endcode
 *
 * Stores the allocator and element count alongside the array allocation.
 *
 * \attention 1. A moved-from array is empty and cannot be resized.
 * \attention 2. Element access requires an index less than size().
 */
template <typename T, Allocator Alloc = Mallocator>
class BoxedArray {
public:
  using value_type = T;
  using allocator_type = std::remove_cvref_t<Alloc>;
  using size_type = size_t;
  using iterator = T *;
  using const_iterator = const T *;

private:
  struct Header {
    size_type size;
    [[no_unique_address]] allocator_type allocator;
  };

  struct alignas(T) alignas(Header) Unit {
    std::byte byte;
  };

  using allocator_traits = AllocatorTraits<allocator_type>;

  static constexpr size_type data_offset =
      sizeof(Header) / alignof(T) * alignof(T) +
      (sizeof(Header) % alignof(T) != 0 ? alignof(T) : 0);

  [[nodiscard]] static constexpr size_type units_for(size_type size) noexcept {
    assert(size <=
           (std::numeric_limits<size_type>::max() - data_offset) / sizeof(T));
    const size_type bytes = data_offset + size * sizeof(T);
    return bytes / sizeof(Unit) + (bytes % sizeof(Unit) != 0);
  }

  [[nodiscard]] static Header *allocate(size_type size,
                                         const allocator_type &alloc) {
    allocator_type local = alloc;
    Unit *block =
        allocator_traits::template allocate<Unit>(local, units_for(size));
    assert(block != nullptr);
    return std::construct_at(reinterpret_cast<Header *>(block), size, local);
  }

  [[nodiscard]] static T *elements(Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }

    auto *bytes = reinterpret_cast<std::byte *>(header);
    return reinterpret_cast<T *>(bytes + data_offset);
  }

  [[nodiscard]] static const T *elements(const Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }

    auto *bytes = reinterpret_cast<const std::byte *>(header);
    return reinterpret_cast<const T *>(bytes + data_offset);
  }

  void destroy() noexcept {
    if (m_header == nullptr) {
      return;
    }

    Header *header = m_header;
    const size_type n = header->size;
    allocator_type alloc = header->allocator;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_type i = 0; i < n; ++i) {
        std::destroy_at(elements(header) + i);
      }
    }

    std::destroy_at(header);
    allocator_traits::template deallocate<Unit>(
        alloc, reinterpret_cast<Unit *>(header), units_for(n));
    m_header = nullptr;
  }

public:
  /**
   * \brief Constructs, copies, moves, assigns, or destroys an array.
   * \code{.cpp}
   * explicit BoxedArray(size_type size, const allocator_type& alloc = {});
   * template<std::ranges::forward_range Range> explicit BoxedArray(Range&& range, const allocator_type& alloc = {});
   * BoxedArray(const BoxedArray& other);
   * BoxedArray(BoxedArray&& other) noexcept;
   * BoxedArray& operator=(const BoxedArray& other);
   * BoxedArray& operator=(BoxedArray&& other) noexcept;
   * ~BoxedArray() noexcept;
   * \endcode
   * \param size Number of elements.
   * \param range Source range.
   * \param alloc Allocator to use.
   * \param other Array to copy or move from.
   * Moved-from arrays are empty and cannot be resized.
   */
  explicit BoxedArray(size_type size, const allocator_type &alloc = {})
    requires std::default_initializable<T>
      : m_header(allocate(size, alloc)) {
    for (size_type i = 0; i < size; ++i) {
      std::construct_at(elements(m_header) + i);
    }
  }

  /**
   * \brief Constructs an array from a range.
   * \code{.cpp}
   * template<std::ranges::forward_range Range>
   * explicit BoxedArray(Range&& range,
   *                     const allocator_type& alloc = {});
   * \endcode
   *
   * \param range Source range.
   * \param alloc Allocator to use.
   *
   * Elements are constructed from the corresponding range values.
   *
   * \attention 1. T must be constructible from the range reference type.
   * \attention 2. Range must be a forward range.
   */
  template <std::ranges::forward_range Range>
    requires std::constructible_from<T,
                                     std::ranges::range_reference_t<Range>> &&
             (!std::same_as<std::remove_cvref_t<Range>, BoxedArray>)
  explicit BoxedArray(Range &&range, const allocator_type &alloc = {})
      : m_header(allocate(static_cast<size_type>(std::ranges::distance(range)),
                          alloc)) {
    size_type i = 0;

    for (auto &&value : range) {
      std::construct_at(elements(m_header) + i,
                        std::forward<decltype(value)>(value));
      ++i;
    }
  }

  BoxedArray(const BoxedArray &other)
    requires std::copy_constructible<T>
  {
    if (other.m_header == nullptr) {
      return;
    }

    m_header = allocate(other.size(), other.m_header->allocator);

    for (size_type i = 0; i < size(); ++i) {
      std::construct_at(elements(m_header) + i, other[i]);
    }
  }

  BoxedArray(BoxedArray &&other) noexcept
      : m_header(std::exchange(other.m_header, nullptr)) {}

  BoxedArray &operator=(const BoxedArray &other)
    requires std::copy_constructible<T>
  {
    if (this == &other) {
      return *this;
    }

    destroy();

    if (other.m_header == nullptr) {
      return *this;
    }

    m_header = allocate(other.size(), other.m_header->allocator);

    for (size_type i = 0; i < size(); ++i) {
      std::construct_at(elements(m_header) + i, other[i]);
    }

    return *this;
  }

  BoxedArray &operator=(BoxedArray &&other) noexcept {
    if (this != &other) {
      destroy();
      m_header = std::exchange(other.m_header, nullptr);
    }

    return *this;
  }

  ~BoxedArray() noexcept { destroy(); }

  /**
   * \brief Changes the number of elements.
   * \code{.cpp}
   * void resize(size_type newSize);
   * \endcode
   *
   * Existing elements are move-constructed into the new allocation.
   *
   * \param size New number of elements.
   *
   * \attention 1. The array must not be moved-from.
   * \attention 2. T must be default-initializable.
   */
  void resize(size_type newSize)
    requires std::default_initializable<T>
  {
    assert(m_header != nullptr);

    const size_type oldSize = size();

    if (newSize == oldSize) {
      return;
    }

    Header *newHeader = allocate(newSize, m_header->allocator);
    T *newData = elements(newHeader);
    T *oldData = data();
    const size_type retained = std::min(oldSize, newSize);

    for (size_type i = 0; i < retained; ++i) {
      std::construct_at(newData + i, std::move(oldData[i]));
    }

    for (size_type i = retained; i < newSize; ++i) {
      std::construct_at(newData + i);
    }

    destroy();
    m_header = newHeader;
  }

  /**
   * \brief Returns the number of elements.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   *
   * \return Number of elements.
   */
  [[nodiscard]] size_type size() const noexcept {
    return m_header == nullptr ? 0 : m_header->size;
  }

  /**
   * \brief Checks whether the array is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   *
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept {
    return size() == 0;
  }

  /**
   * \brief Returns the element storage.
   * \code{.cpp}
   * T* data() noexcept;
   * const T* data() const noexcept;
   * \endcode
   *
   * \return Pointer to the first element, or nullptr for an empty array.
   */
  [[nodiscard]] T *data() noexcept {
    return elements(m_header);
  }

  [[nodiscard]] const T *data() const noexcept {
    return elements(m_header);
  }

  /**
   * \brief Returns an iterator to the first element.
   * \code{.cpp}
   * iterator begin() noexcept;
   * const_iterator begin() const noexcept;
   * \endcode
   *
   * \return Iterator to the first element.
   */
  [[nodiscard]] iterator begin() noexcept {
    return data();
  }

  [[nodiscard]] const_iterator begin() const noexcept {
    return data();
  }

  /**
   * \brief Returns an iterator past the last element.
   * \code{.cpp}
   * iterator end() noexcept;
   * const_iterator end() const noexcept;
   * \endcode
   *
   * \return Iterator past the last element.
   */
  [[nodiscard]] iterator end() noexcept {
    return empty() ? data() : data() + size();
  }

  [[nodiscard]] const_iterator end() const noexcept {
    return empty() ? data() : data() + size();
  }

  /**
   * \brief Accesses an element.
   * \code{.cpp}
   * T& operator[](size_type index) noexcept;
   * const T& operator[](size_type index) const noexcept;
   * \endcode
   *
   * \param index Element index.
   *
   * \return Reference to the element.
   *
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]] T &operator[](size_type index) noexcept {
    assert(index < size());
    return data()[index];
  }

  [[nodiscard]] const T &operator[](size_type index) const noexcept {
    assert(index < size());
    return data()[index];
  }

  /**
   * \brief Accesses the first element.
   * \code{.cpp}
   * T& front() noexcept;
   * const T& front() const noexcept;
   * \endcode
   * \return Reference to the first element.
   * \attention 1. The array must not be empty.
   */
  [[nodiscard]] T &front() noexcept {
    assert(!empty());
    return data()[0];
  }

  [[nodiscard]] const T &front() const noexcept {
    assert(!empty());
    return data()[0];
  }

  /**
   * \brief Accesses the last element.
   * \code{.cpp}
   * T& back() noexcept;
   * const T& back() const noexcept;
   * \endcode
   * \return Reference to the last element.
   * \attention 1. The array must not be empty.
   */
  [[nodiscard]] T &back() noexcept {
    assert(!empty());
    return data()[size() - 1];
  }

  [[nodiscard]] const T &back() const noexcept {
    assert(!empty());
    return data()[size() - 1];
  }

  /**
   * \brief Compares two arrays for equality.
   * \code{.cpp}
   * friend bool operator==(const BoxedArray& lhs, const BoxedArray& rhs);
   * \endcode
   * \param lhs Left array.
   * \param rhs Right array.
   * \return Whether both arrays have equal sizes and equal elements.
   */
  friend bool operator==(const BoxedArray &lhs, const BoxedArray &rhs) {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  /**
   * \brief Compares two arrays for inequality.
   * \code{.cpp}
   * friend bool operator!=(const BoxedArray& lhs, const BoxedArray& rhs);
   * \endcode
   * \param lhs Left array.
   * \param rhs Right array.
   * \return Whether the arrays differ.
   */
  friend bool operator!=(const BoxedArray &lhs, const BoxedArray &rhs) {
    return !(lhs == rhs);
  }

  /**
   * \brief Performs a lexicographical three-way comparison.
   * \code{.cpp}
   * friend auto operator<=>(const BoxedArray& lhs, const BoxedArray& rhs);
   * \endcode
   * \param lhs Left array.
   * \param rhs Right array.
   * \return The lexicographical comparison result.
   */
  friend auto operator<=>(const BoxedArray &lhs, const BoxedArray &rhs) {
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(),
                                                   rhs.begin(), rhs.end());
  }

  /**
   * \brief Returns the array allocator.
   * \code{.cpp}
   * allocator_type get_allocator() const;
   * \endcode
   *
   * \return A copy of the allocator.
   *
   * \attention 1. The array must not be moved-from.
   */
  [[nodiscard]] allocator_type get_allocator() const {
    assert(m_header != nullptr);
    return m_header->allocator;
  }

private:
  Header *m_header = nullptr;
};

} // namespace strobe

namespace std {

template <typename T, strobe::Allocator Alloc>
  requires requires(const T &value) { std::hash<T>{}(value); }
struct hash<strobe::BoxedArray<T, Alloc>> {
  size_t operator()(const strobe::BoxedArray<T, Alloc> &array) const
      noexcept(noexcept(std::hash<T>{}(*array.begin()))) {
    size_t result = static_cast<size_t>(1469598103934665603ull);
    for (const T &value : array) {
      const size_t value_hash = std::hash<T>{}(value);
      result ^= value_hash + static_cast<size_t>(0x9e3779b97f4a7c15ull) +
                (result << 6) + (result >> 2);
    }
    return result;
  }
};

} // namespace std

namespace fmt {

template <typename T, strobe::Allocator Alloc>
struct formatter<strobe::BoxedArray<T, Alloc>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::BoxedArray<T, Alloc> &array,
              FormatContext &ctx) const {
    auto out = ctx.out();
    *out++ = '[';
    for (size_t i = 0; i < array.size(); ++i) {
      if (i != 0) {
        *out++ = ',';
        *out++ = ' ';
      }
      out = fmt::format_to(out, "{}", array[i]);
    }
    *out++ = ']';
    return out;
  }
};

} // namespace fmt
