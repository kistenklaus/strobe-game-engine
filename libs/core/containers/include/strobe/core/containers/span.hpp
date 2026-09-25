#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <fmt/format.h>
#include <ranges>
#include <stdexcept>
#include <type_traits>

namespace strobe {

/**
 * \brief Indicates that a span extent is determined at runtime.
 * \code{.cpp}
 * inline constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);
 * \endcode
 */
inline constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

/**
 * \ingroup core
 * \brief Non-owning view over a contiguous sequence of objects.
 * \code{.cpp}
 * template<typename ElementType, size_t Extent = dynamic_extent>
 * class span;
 * \endcode
 */
template <typename ElementType, std::size_t Extent = dynamic_extent> class span;

namespace detail {

template <typename> struct is_span : std::false_type {};

template <typename T, std::size_t Extent>
struct is_span<span<T, Extent>> : std::true_type {};

template <typename> struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T, std::size_t Extent> class span_storage;

// -----------------------------------------------------------------------------
// Dynamic extent storage
// -----------------------------------------------------------------------------

template <typename T> class span_storage<T, dynamic_extent> {
public:
  constexpr span_storage() noexcept = default;

  constexpr span_storage(T *data, std::size_t size) noexcept
      : m_data(data), m_size(size) {}

  [[nodiscard]]
  constexpr T *data() const noexcept {
    return m_data;
  }

  [[nodiscard]]
  constexpr std::size_t size() const noexcept {
    return m_size;
  }

private:
  T *m_data = nullptr;
  std::size_t m_size = 0;
};

// -----------------------------------------------------------------------------
// Static extent storage
// -----------------------------------------------------------------------------

template <typename T, std::size_t Extent> class span_storage {
public:
  constexpr span_storage() noexcept
    requires(Extent == 0)
  = default;

  constexpr span_storage(T *data, [[maybe_unused]] std::size_t size) noexcept
      : m_data(data) {
    assert(size == Extent);
  }

  [[nodiscard]]
  constexpr T *data() const noexcept {
    return m_data;
  }

  [[nodiscard]]
  static constexpr std::size_t size() noexcept {
    return Extent;
  }

private:
  T *m_data = nullptr;
};

} // namespace detail

template <typename ElementType, std::size_t Extent> class span {
  static_assert(std::is_object_v<ElementType>,
                "span element type must be an object type");

public:
  using element_type = ElementType;
  using value_type = std::remove_cv_t<ElementType>;

  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using pointer = element_type *;
  using const_pointer = const element_type *;

  using reference = element_type &;
  using const_reference = const element_type &;

  using iterator = pointer;
  using const_iterator = const_pointer;

  using reverse_iterator = std::reverse_iterator<iterator>;

  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  static constexpr size_type extent = Extent;

  // ===========================================================================
  // Constructors
  // ===========================================================================

  /**
   * \brief Constructs, copies, or assigns a span.
   * \code{.cpp}
   * constexpr span() noexcept;
   * constexpr span(pointer data) noexcept;
   * template<std::contiguous_iterator It>
   * constexpr span(It first, size_type count) noexcept;
   * template<std::contiguous_iterator It, std::sized_sentinel_for<It> End>
   * constexpr span(It first, End last);
   * template<size_type N>
   * constexpr span(element_type (&array)[N]) noexcept;
   * template<typename T, size_type N>
   * constexpr span(std::array<T, N>& array) noexcept;
   * template<typename T, size_type N>
   * constexpr span(const std::array<T, N>& array) noexcept;
   * template<typename R>
   * constexpr span(R&& range);
   * constexpr span(const span&) noexcept = default;
   * constexpr span& operator=(const span&) noexcept = default;
   * template<typename OtherElementType, size_type OtherExtent>
   * constexpr span(const span<OtherElementType, OtherExtent>& other) noexcept;
   * \endcode
   * Spans do not own their elements; referenced storage must outlive the span.
   */
  constexpr span() noexcept
    requires(Extent == dynamic_extent || Extent == 0)
      : m_storage(nullptr, 0) {}

  constexpr span(pointer data) noexcept
    requires(Extent == dynamic_extent || Extent == 1)
      : m_storage(data, data != nullptr ? 1 : 0) {}

  template <std::contiguous_iterator It>
    requires(std::is_convertible_v<
             std::remove_reference_t<std::iter_reference_t<It>> (*)[],
             element_type (*)[]>)
  constexpr explicit(Extent != dynamic_extent)
      span(It first, size_type count) noexcept
      : m_storage(std::to_address(first), count) {}

  template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
    requires(!std::is_convertible_v<End, size_type> &&
             std::is_convertible_v<
                 std::remove_reference_t<std::iter_reference_t<It>> (*)[],
                 element_type (*)[]>)
  constexpr explicit(Extent != dynamic_extent) span(It first, End last)
      : m_storage(std::to_address(first),
                  static_cast<size_type>(last - first)) {}

  template <size_type N>
    requires(Extent == dynamic_extent || Extent == N)
  constexpr span(std::type_identity_t<element_type> (&array)[N]) noexcept
      : m_storage(array, N) {}

  template <typename T, size_type N>
    requires((Extent == dynamic_extent || Extent == N) &&
             std::is_convertible_v<T (*)[], element_type (*)[]>)
  constexpr span(std::array<T, N> &array) noexcept
      : m_storage(array.data(), N) {}

  template <typename T, size_type N>
    requires((Extent == dynamic_extent || Extent == N) &&
             std::is_convertible_v<const T (*)[], element_type (*)[]>)
  constexpr span(const std::array<T, N> &array) noexcept
      : m_storage(array.data(), N) {}

  template <typename R>
    requires(
        std::ranges::contiguous_range<R> && std::ranges::sized_range<R> &&
        (std::ranges::borrowed_range<R> || std::is_const_v<element_type>) &&
        !detail::is_span<std::remove_cvref_t<R>>::value &&
        !detail::is_std_array<std::remove_cvref_t<R>>::value &&
        !std::is_array_v<std::remove_cvref_t<R>> &&
        std::is_convertible_v<
            std::remove_reference_t<std::ranges::range_reference_t<R>> (*)[],
            element_type (*)[]>)
  constexpr explicit(Extent != dynamic_extent) span(R &&range)
      : m_storage(std::ranges::data(range),
                  static_cast<size_type>(std::ranges::size(range))) {}

  constexpr span(const span &) noexcept = default;

  constexpr span &operator=(const span &) noexcept = default;

  template <typename OtherElementType, size_type OtherExtent>
    requires((Extent == dynamic_extent || OtherExtent == dynamic_extent ||
              Extent == OtherExtent) &&
             std::is_convertible_v<OtherElementType (*)[], element_type (*)[]>)
  constexpr explicit(Extent != dynamic_extent && OtherExtent == dynamic_extent)
      span(const span<OtherElementType, OtherExtent> &other) noexcept
      : m_storage(other.data(), other.size()) {}

  // ===========================================================================
  // Compile-time subviews
  // ===========================================================================

  /**
   * \brief Returns a fixed-size prefix view.
   * \code{.cpp}
   * template<size_type Count>
   * constexpr span<element_type, Count> first() const;
   * \endcode
   * \tparam Count Number of elements.
   * \return Prefix view.
   */
  template <size_type Count>
  [[nodiscard]]
  constexpr span<element_type, Count> first() const {
    static_assert(Extent == dynamic_extent || Count <= Extent);

    assert(Count <= size());

    return span<element_type, Count>(data(), Count);
  }

  /**
   * \brief Returns a fixed-size suffix view.
   * \code{.cpp}
   * template<size_type Count>
   * constexpr span<element_type, Count> last() const;
   * \endcode
   * \tparam Count Number of elements.
   * \return Suffix view.
   */
  template <size_type Count>
  [[nodiscard]]
  constexpr span<element_type, Count> last() const {
    static_assert(Extent == dynamic_extent || Count <= Extent);

    assert(Count <= size());

    return span<element_type, Count>(data() + size() - Count, Count);
  }

  /**
   * \brief Returns a fixed-size subview.
   * \code{.cpp}
   * template<size_type Offset, size_type Count = dynamic_extent>
   * constexpr auto subspan() const;
   * \endcode
   * \return Requested subview.
   */
  template <size_type Offset, size_type Count = dynamic_extent>
  [[nodiscard]]
  constexpr auto subspan() const {
    static_assert(Extent == dynamic_extent || Offset <= Extent);

    static_assert(Count == dynamic_extent || Extent == dynamic_extent ||
                  Count <= Extent - Offset);

    assert(Offset <= size());

    if constexpr (Count != dynamic_extent) {
      assert(Count <= size() - Offset);

      return span<element_type, Count>(data() + Offset, Count);
    } else {
      constexpr size_type NewExtent =
          Extent == dynamic_extent ? dynamic_extent : Extent - Offset;

      return span<element_type, NewExtent>(data() + Offset, size() - Offset);
    }
  }

  // ===========================================================================
  // Runtime subviews
  // ===========================================================================

  /**
   * \brief Returns a runtime prefix view.
   * \code{.cpp}
   * constexpr span<element_type, dynamic_extent> first(size_type count) const;
   * \endcode
   * \param count Number of elements.
   * \return Prefix view.
   */
  [[nodiscard]]
  constexpr span<element_type, dynamic_extent> first(size_type count) const {
    assert(count <= size());

    return {
        data(),
        count,
    };
  }

  /**
   * \brief Returns a runtime suffix view.
   * \code{.cpp}
   * constexpr span<element_type, dynamic_extent> last(size_type count) const;
   * \endcode
   * \param count Number of elements.
   * \return Suffix view.
   */
  [[nodiscard]]
  constexpr span<element_type, dynamic_extent> last(size_type count) const {
    assert(count <= size());

    return {
        data() + size() - count,
        count,
    };
  }

  /**
   * \brief Returns a runtime subview.
   * \code{.cpp}
   * constexpr span<element_type, dynamic_extent> subspan(
   *     size_type offset, size_type count = dynamic_extent) const;
   * \endcode
   * \param offset First element offset.
   * \param count Number of elements.
   * \return Requested subview.
   */
  [[nodiscard]]
  constexpr span<element_type, dynamic_extent>
  subspan(size_type offset, size_type count = dynamic_extent) const {

    assert(offset <= size());

    const size_type actualCount =
        count == dynamic_extent ? size() - offset : count;

    assert(actualCount <= size() - offset);

    return {
        data() + offset,
        actualCount,
    };
  }

  // ===========================================================================
  // Observers
  // ===========================================================================

  /**
   * \brief Returns the number of elements.
   * \code{.cpp}
   * constexpr size_type size() const noexcept;
   * \endcode
   * \return Number of elements.
   */
  [[nodiscard]]
  constexpr size_type size() const noexcept {
    return m_storage.size();
  }

  /**
   * \brief Returns the size in bytes.
   * \code{.cpp}
   * constexpr size_type size_bytes() const noexcept;
   * \endcode
   * \return Number of bytes.
   */
  [[nodiscard]]
  constexpr size_type size_bytes() const noexcept {
    return size() * sizeof(element_type);
  }

  /**
   * \brief Checks whether the span is empty.
   * \code{.cpp}
   * constexpr bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]]
  constexpr bool empty() const noexcept {
    return size() == 0;
  }

  // ===========================================================================
  // Element access
  // ===========================================================================

  /**
   * \brief Accesses an element without bounds checking.
   * \code{.cpp}
   * constexpr reference operator[](size_type index) const;
   * \endcode
   * \param index Element index.
   * \return Reference to the element.
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]]
  constexpr reference operator[](size_type index) const {
    assert(index < size());
    return data()[index];
  }

  /**
   * \brief Bounds-checks and accesses an element.
   * \code{.cpp}
   * constexpr reference at(size_type index) const;
   * \endcode
   * \param index Element index.
   * \return Reference to the element.
   * \throws std::out_of_range when \p index >= size().
   */
  [[nodiscard]]
  constexpr reference at(size_type index) const {
    if (index >= size()) {
      throw std::out_of_range("strobe::span::at");
    }

    return data()[index];
  }

  /**
   * \brief Returns the first element.
   * \code{.cpp}
   * constexpr reference front() const;
   * \endcode
   * \return Reference to the first element.
   */
  [[nodiscard]]
  constexpr reference front() const {
    assert(!empty());
    return data()[0];
  }

  /**
   * \brief Returns the last element.
   * \code{.cpp}
   * constexpr reference back() const;
   * \endcode
   * \return Reference to the last element.
   */
  [[nodiscard]]
  constexpr reference back() const {
    assert(!empty());
    return data()[size() - 1];
  }

  /**
   * \brief Returns the element storage.
   * \code{.cpp}
   * constexpr pointer data() const noexcept;
   * \endcode
   * \return Pointer to the first element.
   */
  [[nodiscard]]
  constexpr pointer data() const noexcept {
    return m_storage.data();
  }

  // ===========================================================================
  // Iterators
  // ===========================================================================

  /**
   * \brief Returns an iterator to the first element.
   * \code{.cpp}
   * constexpr iterator begin() const noexcept;
   * \endcode
   * \return Iterator to the first element.
   */
  [[nodiscard]]
  constexpr iterator begin() const noexcept {
    return data();
  }

  /**
   * \brief Returns an iterator past the last element.
   * \code{.cpp}
   * constexpr iterator end() const noexcept;
   * \endcode
   * \return Iterator past the last element.
   */
  [[nodiscard]]
  constexpr iterator end() const noexcept {
    return data() + size();
  }

  /**
   * \brief Returns a const iterator to the first element.
   * \code{.cpp}
   * constexpr const_iterator cbegin() const noexcept;
   * \endcode
   * \return Const iterator to the first element.
   */
  [[nodiscard]]
  constexpr const_iterator cbegin() const noexcept {
    return data();
  }

  /**
   * \brief Returns a const iterator past the last element.
   * \code{.cpp}
   * constexpr const_iterator cend() const noexcept;
   * \endcode
   * \return Const iterator past the last element.
   */
  [[nodiscard]]
  constexpr const_iterator cend() const noexcept {
    return data() + size();
  }

  /**
   * \brief Returns a reverse iterator to the last element.
   * \code{.cpp}
   * constexpr reverse_iterator rbegin() const noexcept;
   * \endcode
   * \return Reverse iterator to the last element.
   */
  [[nodiscard]]
  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }

  /**
   * \brief Returns a reverse iterator before the first element.
   * \code{.cpp}
   * constexpr reverse_iterator rend() const noexcept;
   * \endcode
   * \return Reverse iterator before the first element.
   */
  [[nodiscard]]
  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  }

  /**
   * \brief Returns a const reverse iterator to the last element.
   * \code{.cpp}
   * constexpr const_reverse_iterator crbegin() const noexcept;
   * \endcode
   * \return Const reverse iterator to the last element.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

  /**
   * \brief Returns a const reverse iterator before the first element.
   * \code{.cpp}
   * constexpr const_reverse_iterator crend() const noexcept;
   * \endcode
   * \return Const reverse iterator before the first element.
   */
  [[nodiscard]]
  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

private:
  detail::span_storage<element_type, Extent> m_storage;
};

// ===========================================================================
// Deduction guides
// ===========================================================================

template <std::contiguous_iterator It, typename EndOrSize>
span(It, EndOrSize) -> span<std::remove_reference_t<std::iter_reference_t<It>>>;

template <typename T, std::size_t N> span(T (&)[N]) -> span<T, N>;

template <typename T, std::size_t N> span(std::array<T, N> &) -> span<T, N>;

template <typename T, std::size_t N>
span(const std::array<T, N> &) -> span<const T, N>;

template <std::ranges::contiguous_range R>
span(R &&) -> span<std::remove_reference_t<std::ranges::range_reference_t<R>>>;

// ===========================================================================
// Object representation
// ===========================================================================

/**
 * \brief Views the object representation as read-only bytes.
 * \code{.cpp}
 * template<typename T, size_t Extent>
 * constexpr auto as_bytes(span<T, Extent> value) noexcept;
 * \endcode
 * \param value Span whose representation is viewed.
 * \return Read-only byte span over the same storage.
 */
template <typename T, std::size_t Extent>
[[nodiscard]]
constexpr auto as_bytes(span<T, Extent> s) noexcept {

  constexpr std::size_t ByteExtent =
      Extent == dynamic_extent ? dynamic_extent : sizeof(T) * Extent;

  return span<const std::byte, ByteExtent>(
      reinterpret_cast<const std::byte *>(s.data()), s.size_bytes());
}

/**
 * \brief Views the object representation as writable bytes.
 * \code{.cpp}
 * template<typename T, size_t Extent>
 * constexpr auto as_writable_bytes(span<T, Extent> value) noexcept;
 * \endcode
 * \param value Span whose representation is viewed.
 * \return Writable byte span over the same storage.
 * \attention 1. The element type must not be const.
 */
template <typename T, std::size_t Extent>
  requires(!std::is_const_v<T>)
[[nodiscard]]
constexpr auto as_writable_bytes(span<T, Extent> s) noexcept {

  constexpr std::size_t ByteExtent =
      Extent == dynamic_extent ? dynamic_extent : sizeof(T) * Extent;

  return span<std::byte, ByteExtent>(reinterpret_cast<std::byte *>(s.data()),
                                     s.size_bytes());
}

} // namespace strobe

namespace fmt {

/**
 * \brief Formats a span as a bracketed list of elements.
 * \code{.cpp}
 * template<typename ElementType, size_t Extent>
 * struct formatter<strobe::span<ElementType, Extent>>;
 * \endcode
 */
template <typename ElementType, std::size_t Extent>
struct formatter<strobe::span<ElementType, Extent>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::span<ElementType, Extent> &value,
              FormatContext &ctx) const {
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
