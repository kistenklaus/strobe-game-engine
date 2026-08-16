#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace strobe {

inline constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

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

  constexpr span_storage(T *data, std::size_t size) noexcept : m_data(data) {
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

  constexpr span() noexcept
    requires(Extent == dynamic_extent || Extent == 0)
      : m_storage(nullptr, 0) {}

  constexpr span(pointer data) noexcept
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

  template <size_type Count>
  [[nodiscard]]
  constexpr span<element_type, Count> first() const {
    static_assert(Extent == dynamic_extent || Count <= Extent);

    assert(Count <= size());

    return span<element_type, Count>(data(), Count);
  }

  template <size_type Count>
  [[nodiscard]]
  constexpr span<element_type, Count> last() const {
    static_assert(Extent == dynamic_extent || Count <= Extent);

    assert(Count <= size());

    return span<element_type, Count>(data() + size() - Count, Count);
  }

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

  [[nodiscard]]
  constexpr span<element_type, dynamic_extent> first(size_type count) const {
    assert(count <= size());

    return {
        data(),
        count,
    };
  }

  [[nodiscard]]
  constexpr span<element_type, dynamic_extent> last(size_type count) const {
    assert(count <= size());

    return {
        data() + size() - count,
        count,
    };
  }

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

  [[nodiscard]]
  constexpr size_type size() const noexcept {
    return m_storage.size();
  }

  [[nodiscard]]
  constexpr size_type size_bytes() const noexcept {
    return size() * sizeof(element_type);
  }

  [[nodiscard]]
  constexpr bool empty() const noexcept {
    return size() == 0;
  }

  // ===========================================================================
  // Element access
  // ===========================================================================

  [[nodiscard]]
  constexpr reference operator[](size_type index) const {
    assert(index < size());
    return data()[index];
  }

  [[nodiscard]]
  constexpr reference at(size_type index) const {
    if (index >= size()) {
      throw std::out_of_range("strobe::span::at");
    }

    return data()[index];
  }

  [[nodiscard]]
  constexpr reference front() const {
    assert(!empty());
    return data()[0];
  }

  [[nodiscard]]
  constexpr reference back() const {
    assert(!empty());
    return data()[size() - 1];
  }

  [[nodiscard]]
  constexpr pointer data() const noexcept {
    return m_storage.data();
  }

  // ===========================================================================
  // Iterators
  // ===========================================================================

  [[nodiscard]]
  constexpr iterator begin() const noexcept {
    return data();
  }

  [[nodiscard]]
  constexpr iterator end() const noexcept {
    return data() + size();
  }

  [[nodiscard]]
  constexpr const_iterator cbegin() const noexcept {
    return data();
  }

  [[nodiscard]]
  constexpr const_iterator cend() const noexcept {
    return data() + size();
  }

  [[nodiscard]]
  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }

  [[nodiscard]]
  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  }

  [[nodiscard]]
  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }

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

template <typename T, std::size_t Extent>
[[nodiscard]]
constexpr auto as_bytes(span<T, Extent> s) noexcept {

  constexpr std::size_t ByteExtent =
      Extent == dynamic_extent ? dynamic_extent : sizeof(T) * Extent;

  return span<const std::byte, ByteExtent>(
      reinterpret_cast<const std::byte *>(s.data()), s.size_bytes());
}

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
