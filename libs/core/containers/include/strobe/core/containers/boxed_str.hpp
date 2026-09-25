#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <cassert>
#include <cstddef>
#include <functional>
#include <fmt/format.h>
#include <iterator>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Heap-allocated null-terminated string.
 * \code{.cpp}
 * template<Allocator A = Mallocator, typename C = char>
 * class BoxedStr;
 * \endcode
 *
 * Provides a read-only string interface backed by a single allocation
 * containing the metadata and character storage.
 *
 * \attention 1. The stored character sequence is always null-terminated.
 * \attention 2. Element access requires an index less than or equal to size().
 * \attention 3. A moved-from string is empty and cannot be used with
 * get_allocator().
 */
template <Allocator A = Mallocator, typename C = char>
class BoxedStr {
public:
  using value_type = C;
  using traits_type = std::char_traits<C>;
  using allocator_type = std::remove_cvref_t<A>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_reference = const C &;
  using const_pointer = const C *;
  using const_iterator = const C *;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using view_type = std::basic_string_view<C, traits_type>;

  static constexpr size_type npos = size_type(-1);

private:
  struct Header {
    size_type size;
    [[no_unique_address]] allocator_type allocator;
  };

  struct alignas(Header) alignas(C) Unit {
    std::byte byte;
  };

  using allocator_traits = AllocatorTraits<allocator_type>;

  static constexpr size_type data_offset =
      sizeof(Header) / alignof(C) * alignof(C) +
      (sizeof(Header) % alignof(C) != 0 ? alignof(C) : 0);

  [[nodiscard]] static constexpr size_type units_for(size_type size) noexcept {
    assert(size < (std::numeric_limits<size_type>::max() - data_offset) /
                      sizeof(C));

    const size_type bytes = data_offset + (size + 1) * sizeof(C);
    return bytes / sizeof(Unit) + (bytes % sizeof(Unit) != 0);
  }

  [[nodiscard]] static Header *allocate(size_type size,
                                         const allocator_type &alloc) {
    allocator_type local = alloc;
    Unit *block =
        allocator_traits::template allocate<Unit>(local, units_for(size));
    assert(block != nullptr);

    Header *header =
        std::construct_at(reinterpret_cast<Header *>(block), size, local);

    C *chars = elements(header);
    for (size_type i = 0; i <= size; ++i) {
      std::construct_at(chars + i, C{});
    }

    return header;
  }

  [[nodiscard]] static C *elements(Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }

    auto *bytes = reinterpret_cast<std::byte *>(header);
    return reinterpret_cast<C *>(bytes + data_offset);
  }

  [[nodiscard]] static const C *elements(const Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }

    auto *bytes = reinterpret_cast<const std::byte *>(header);
    return reinterpret_cast<const C *>(bytes + data_offset);
  }

  void destroy() noexcept {
    if (m_header == nullptr) {
      return;
    }

    Header *header = m_header;
    const size_type count = header->size;
    allocator_type alloc = header->allocator;

    for (size_type i = 0; i <= count; ++i) {
      std::destroy_at(elements(header) + i);
    }

    std::destroy_at(header);
    allocator_traits::template deallocate<Unit>(
        alloc, reinterpret_cast<Unit *>(header), units_for(count));
    m_header = nullptr;
  }

public:
  /**
   * \brief Constructs a string.
   * \code{.cpp}
   * explicit BoxedStr(size_type size, const allocator_type& alloc = {});
   * explicit BoxedStr(view_type value, const allocator_type& alloc = {});
   * BoxedStr(const C* value, const allocator_type& alloc = {});
   * \endcode
   *
   * \param size Number of characters.
   * \param value Source string.
   * \param alloc Allocator to use.
   *
   * The size constructor initializes all characters to C{}.
   *
   * \attention 1. The pointer constructor requires a non-null,
   * null-terminated pointer.
   */
  explicit BoxedStr(size_type size, const allocator_type &alloc = {})
      : m_header(allocate(size, alloc)) {}

  explicit BoxedStr(view_type value, const allocator_type &alloc = {})
      : m_header(allocate(value.size(), alloc)) {
    if (!value.empty()) {
      traits_type::copy(elements(m_header), value.data(), value.size());
    }
  }

  BoxedStr(const C *value, const allocator_type &alloc = {})
      : BoxedStr(view_type(value), alloc) {}

  /**
   * \brief Copies or moves a string.
   * \code{.cpp}
   * BoxedStr(const BoxedStr& other);
   * BoxedStr(BoxedStr&& other) noexcept;
   * BoxedStr& operator=(const BoxedStr& other);
   * BoxedStr& operator=(BoxedStr&& other) noexcept;
   * \endcode
   *
   * \param other String to copy or move from.
   *
   * \return This string for assignment.
   */
  BoxedStr(const BoxedStr &other) {
    if (other.m_header == nullptr) {
      return;
    }

    m_header = allocate(other.size(), other.get_allocator());

    if (!other.empty()) {
      traits_type::copy(elements(m_header), other.data(), other.size());
    }
  }

  BoxedStr(BoxedStr &&other) noexcept
      : m_header(std::exchange(other.m_header, nullptr)) {}

  BoxedStr &operator=(const BoxedStr &other) {
    if (this == &other) {
      return *this;
    }

    destroy();

    if (other.m_header == nullptr) {
      return *this;
    }

    m_header = allocate(other.size(), other.get_allocator());

    if (!other.empty()) {
      traits_type::copy(elements(m_header), other.data(), other.size());
    }

    return *this;
  }

  BoxedStr &operator=(BoxedStr &&other) noexcept {
    if (this != &other) {
      destroy();
      m_header = std::exchange(other.m_header, nullptr);
    }

    return *this;
  }

  /**
   * \brief Destroys the string and releases its storage.
   * \code{.cpp}
   * ~BoxedStr() noexcept;
   * \endcode
   */
  ~BoxedStr() noexcept { destroy(); }

  /**
   * \brief Returns the string size.
   * \code{.cpp}
   * size_type size() const noexcept;
   * \endcode
   *
   * \return Number of characters.
   */
  [[nodiscard]] size_type size() const noexcept {
    return m_header == nullptr ? 0 : m_header->size;
  }

  /**
   * \brief Returns the string length.
   * \code{.cpp}
   * [[nodiscard]] size_type length() const noexcept;
   * \endcode
   */
  [[nodiscard]] size_type length() const noexcept {
    return size();
  }

  /**
   * \brief Checks whether the string is empty.
   * \code{.cpp}
   * [[nodiscard]] bool empty() const noexcept;
   * \endcode
   */
  [[nodiscard]] bool empty() const noexcept {
    return size() == 0;
  }

  /**
   * \brief Returns the character storage.
   * \code{.cpp}
   * const_pointer data() const noexcept;
   * \endcode
   *
   * \return A null-terminated character pointer.
   */
  [[nodiscard]] const_pointer data() const noexcept {
    return elements(m_header);
  }

  /**
   * \brief Returns the null-terminated character pointer.
   * \code{.cpp}
   * [[nodiscard]] const_pointer c_str() const noexcept;
   * \endcode
   */
  [[nodiscard]] const_pointer c_str() const noexcept {
    return data();
  }

  /**
   * \brief Returns iterators to the characters.
   * \code{.cpp}
   * const_iterator begin() const noexcept;
   * \endcode
   *
   * \return Iterator delimiting the character sequence.
   */
  [[nodiscard]] const_iterator begin() const noexcept {
    return data();
  }

  /**
   * \brief Returns the end iterator.
   * \code{.cpp}
   * [[nodiscard]] const_iterator end() const noexcept;
   * \endcode
   */
  [[nodiscard]] const_iterator end() const noexcept {
    return empty() ? data() : data() + size();
  }

  /**
   * \brief Returns the begin const iterator.
   * \code{.cpp}
   * [[nodiscard]] const_iterator cbegin() const noexcept;
   * \endcode
   */
  [[nodiscard]] const_iterator cbegin() const noexcept {
    return begin();
  }

  /**
   * \brief Returns the end const iterator.
   * \code{.cpp}
   * [[nodiscard]] const_iterator cend() const noexcept;
   * \endcode
   */
  [[nodiscard]] const_iterator cend() const noexcept {
    return end();
  }

  /**
   * \brief Returns the reverse-begin iterator.
   * \code{.cpp}
   * [[nodiscard]] const_reverse_iterator rbegin() const noexcept;
   * \endcode
   */
  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  /**
   * \brief Returns the reverse-end iterator.
   * \code{.cpp}
   * [[nodiscard]] const_reverse_iterator rend() const noexcept;
   * \endcode
   */
  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /**
   * \brief Accesses a character.
   * \code{.cpp}
   * const_reference operator[](size_type index) const noexcept;
   * \endcode
   *
   * \param index Character index.
   *
   * \return Reference to the character.
   *
   * \attention 1. \p index must be less than or equal to size().
   */
  [[nodiscard]] const_reference
  operator[](size_type index) const noexcept {
    assert(index <= size());
    assert(m_header != nullptr);
    return data()[index];
  }

  /**
   * \brief Returns the first character.
   * \code{.cpp}
   * const_reference front() const noexcept;
   * \endcode
   *
   * \return Reference to the requested character.
   *
   * \attention 1. The string must not be empty.
   */
  [[nodiscard]] const_reference front() const noexcept {
    assert(!empty());
    return data()[0];
  }

  /**
   * \brief Returns the last character.
   * \code{.cpp}
   * [[nodiscard]] const_reference back() const noexcept;
   * \endcode
   */
  [[nodiscard]] const_reference back() const noexcept {
    assert(!empty());
    return data()[size() - 1];
  }

  /**
   * \brief Returns a string view.
   * \code{.cpp}
   * view_type view() const noexcept;
   * \endcode
   *
   * \return A view of the character sequence.
   */
  [[nodiscard]] view_type view() const noexcept {
    return {data(), size()};
  }

  /**
   * \brief Converts to a string view.
   * \code{.cpp}
   * [[nodiscard]] operator view_type() const noexcept;
   * \endcode
   */
  [[nodiscard]] operator view_type() const noexcept {
    return view();
  }

  /**
   * \brief Compares the string with another view.
   * \code{.cpp}
   * int compare(view_type other) const noexcept;
   * \endcode
   *
   * \param other String view to compare with.
   *
   * \return A comparison result following string_view::compare().
   */
  [[nodiscard]] int compare(view_type other) const noexcept {
    return view().compare(other);
  }

  /**
   * \brief Searches for a value.
   * \code{.cpp}
   * size_type find(view_type value, size_type pos = 0) const noexcept;
   * \endcode
   *
   * \param value Value to search for.
   * \param pos Search position.
   *
   * \return Matching position, or npos if no match exists.
   */
  [[nodiscard]] size_type find(view_type value,
                               size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] size_type find(C value,
                               size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  /**
   * \brief Searches backwards for a view.
   * \code{.cpp}
   * [[nodiscard]] size_type rfind(view_type value,;
   * \endcode
   */
  [[nodiscard]] size_type rfind(view_type value,
                                size_type pos = npos) const noexcept {
    return view().rfind(value, pos);
  }

  /**
   * \brief Checks a string prefix or suffix.
   * \code{.cpp}
   * bool starts_with(view_type value) const noexcept;
   * \endcode
   *
   * \param value Value to check.
   *
   * \return Whether the requested relationship holds.
   */
  [[nodiscard]] bool starts_with(view_type value) const noexcept {
    return view().starts_with(value);
  }

  /**
   * \brief Checks the string suffix.
   * \code{.cpp}
   * [[nodiscard]] bool ends_with(view_type value) const noexcept;
   * \endcode
   */
  [[nodiscard]] bool ends_with(view_type value) const noexcept {
    return view().ends_with(value);
  }

  /**
   * \brief Checks whether a value occurs.
   * \code{.cpp}
   * [[nodiscard]] bool contains(view_type value) const noexcept;
   * \endcode
   */
  [[nodiscard]] bool contains(view_type value) const noexcept {
    return find(value) != npos;
  }

  /**
   * \brief Returns a substring view.
   * \code{.cpp}
   * view_type substr(size_type pos = 0, size_type count = npos) const;
   * \endcode
   *
   * \param pos Starting position.
   * \param count Maximum length.
   *
   * \return The requested substring.
   *
   * \attention 1. \p pos must be less than or equal to size().
   */
  [[nodiscard]] view_type substr(size_type pos = 0,
                                 size_type count = npos) const {
    assert(pos <= size());
    return view().substr(pos, count);
  }

  /**
   * \brief Returns the string allocator.
   * \code{.cpp}
   * allocator_type get_allocator() const;
   * \endcode
   *
   * \return A copy of the allocator.
   *
   * \attention 1. The string must not be moved-from.
   */
  [[nodiscard]] allocator_type get_allocator() const {
    assert(m_header != nullptr);
    return m_header->allocator;
  }

  /**
   * \brief Compares two strings for equality.
   * \code{.cpp}
   * bool operator==(const BoxedStr& lhs, const BoxedStr& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return Whether the strings are equal.
   */
  [[nodiscard]] friend bool operator==(const BoxedStr &lhs,
                                       const BoxedStr &rhs) noexcept {
    return lhs.view() == rhs.view();
  }

  /**
   * \brief Compares two strings lexically.
   * \code{.cpp}
   * auto operator<=>(const BoxedStr& lhs, const BoxedStr& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The lexical ordering.
   */
  [[nodiscard]] friend auto operator<=>(const BoxedStr &lhs,
                                        const BoxedStr &rhs) noexcept {
    return lhs.view() <=> rhs.view();
  }

private:
  Header *m_header = nullptr;
};

} // namespace strobe

namespace std {

template <strobe::Allocator Alloc, typename C>
struct hash<strobe::BoxedStr<Alloc, C>> {
  size_t operator()(const strobe::BoxedStr<Alloc, C> &value) const
      noexcept(noexcept(std::hash<C>{}(*value.begin()))) {
    size_t result = static_cast<size_t>(1469598103934665603ull);
    for (C character : value) {
      result ^= std::hash<C>{}(character) +
                static_cast<size_t>(0x9e3779b97f4a7c15ull) +
                (result << 6) + (result >> 2);
    }
    return result;
  }
};

} // namespace std

namespace fmt {

template <strobe::Allocator Alloc, typename C>
struct formatter<strobe::BoxedStr<Alloc, C>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::BoxedStr<Alloc, C> &value,
              FormatContext &ctx) const {
    return fmt::format_to(ctx.out(), "{}", value.view());
  }
};

} // namespace fmt
