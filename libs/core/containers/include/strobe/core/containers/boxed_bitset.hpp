#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <fmt/format.h>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Heap-allocated bitset with a runtime size.
 * \code{.cpp}
 * template<typename Word = uint64_t, Allocator Alloc = Mallocator>
 * class BoxedBitset;
 * \endcode
 *
 * Stores a dynamically sized sequence of bits in unsigned words.
 *
 * \attention 1. \p Word must be an unsigned integer type other than bool.
 * \attention 2. Bits outside the current size are always clear.
 * \attention 3. Bit indices must be less than size().
 * \attention 4. Bitwise operations require equal bitset sizes.
 */
template <typename Word = uint64_t, Allocator Alloc = Mallocator>
  requires(std::unsigned_integral<Word> && !std::same_as<Word, bool>)
class BoxedBitset {
public:
  using word_type = Word;
  using allocator_type = std::remove_cvref_t<Alloc>;

  static constexpr size_t word_bits = std::numeric_limits<word_type>::digits;

private:
  struct Header {
    size_t size;
    [[no_unique_address]] allocator_type allocator;
  };

  struct alignas(Header) alignas(word_type) Unit {
    std::byte byte;
  };

  using allocator_traits = AllocatorTraits<allocator_type>;

  static constexpr word_type full_word = std::numeric_limits<word_type>::max();

  static constexpr size_t words_offset =
      sizeof(Header) / alignof(word_type) * alignof(word_type) +
      (sizeof(Header) % alignof(word_type) != 0 ? alignof(word_type) : 0);

  [[nodiscard]] static constexpr size_t words_for(size_t bits) noexcept {
    return bits / word_bits + (bits % word_bits != 0);
  }

  [[nodiscard]] static constexpr size_t units_for(size_t bits) noexcept {
    const size_t count = words_for(bits);
    assert(count <= (std::numeric_limits<size_t>::max() - words_offset) /
                        sizeof(word_type));

    const size_t bytes = words_offset + count * sizeof(word_type);
    return bytes / sizeof(Unit) + (bytes % sizeof(Unit) != 0);
  }

  [[nodiscard]] static constexpr word_type low_mask(size_t bits) noexcept {
    assert(bits <= word_bits);

    if (bits == word_bits) {
      return full_word;
    }

    if (bits == 0) {
      return 0;
    }

    return static_cast<word_type>((word_type{1} << bits) - word_type{1});
  }

  [[nodiscard]] static Header *allocate(size_t size,
                                         const allocator_type &alloc) {
    allocator_type local = alloc;
    Unit *block =
        allocator_traits::template allocate<Unit>(local, units_for(size));
    assert(block != nullptr);

    return std::construct_at(reinterpret_cast<Header *>(block), size, local);
  }

  [[nodiscard]] static word_type *words(Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }

    auto *bytes = reinterpret_cast<std::byte *>(header);
    return reinterpret_cast<word_type *>(bytes + words_offset);
  }

  [[nodiscard]] static const word_type *words(const Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }

    auto *bytes = reinterpret_cast<const std::byte *>(header);
    return reinterpret_cast<const word_type *>(bytes + words_offset);
  }

  [[nodiscard]] word_type last_mask() const noexcept {
    const size_t tail = size() % word_bits;
    return tail == 0 ? full_word : low_mask(tail);
  }

  void release() noexcept {
    if (m_header == nullptr) {
      return;
    }

    Header *header = m_header;
    const size_t bits = header->size;
    allocator_type alloc = header->allocator;

    std::destroy_at(header);
    allocator_traits::template deallocate<Unit>(
        alloc, reinterpret_cast<Unit *>(header), units_for(bits));
    m_header = nullptr;
  }

public:
  /**
   * \brief Constructs, copies, moves, assigns, or destroys a bitset.
   * \code{.cpp}
   * explicit BoxedBitset(size_t size, const allocator_type& alloc = {});
   * BoxedBitset(const BoxedBitset& other);
   * BoxedBitset(BoxedBitset&& other) noexcept;
   * BoxedBitset& operator=(const BoxedBitset& other);
   * BoxedBitset& operator=(BoxedBitset&& other) noexcept;
   * ~BoxedBitset() noexcept;
   * \endcode
   * \param size Number of bits.
   * \param alloc Allocator to use.
   * \param other Bitset to copy or move from.
   * Moved-from bitsets are empty.
   */
  explicit BoxedBitset(size_t size, const allocator_type &alloc = {})
      : m_header(allocate(size, alloc)) {
    for (size_t i = 0; i < word_count(); ++i) {
      std::construct_at(words(m_header) + i, word_type{0});
    }
  }

  /**
   * \brief Constructs a bitset with every bit set.
   * \code{.cpp}
   * static BoxedBitset full(size_t size,
   *                         const allocator_type& alloc = {});
   * \endcode
   *
   * \param size Number of bits.
   * \param alloc Allocator to use.
   *
   * \return A full bitset.
   */
  [[nodiscard]] static BoxedBitset full(
      size_t size, const allocator_type &alloc = {}) {
    BoxedBitset result(size, alloc);
    result.set_all();
    return result;
  }

  BoxedBitset(const BoxedBitset &other) {
    if (other.m_header == nullptr) {
      return;
    }

    m_header = allocate(other.size(), other.m_header->allocator);

    for (size_t i = 0; i < word_count(); ++i) {
      std::construct_at(words(m_header) + i, other.word(i));
    }
  }

  BoxedBitset(BoxedBitset &&other) noexcept
      : m_header(std::exchange(other.m_header, nullptr)) {}

  BoxedBitset &operator=(const BoxedBitset &other) {
    if (this == &other) {
      return *this;
    }

    release();

    if (other.m_header == nullptr) {
      return *this;
    }

    m_header = allocate(other.size(), other.m_header->allocator);

    for (size_t i = 0; i < word_count(); ++i) {
      std::construct_at(words(m_header) + i, other.word(i));
    }

    return *this;
  }

  BoxedBitset &operator=(BoxedBitset &&other) noexcept {
    if (this != &other) {
      release();
      m_header = std::exchange(other.m_header, nullptr);
    }

    return *this;
  }

  ~BoxedBitset() noexcept { release(); }

  /**
   * \brief Returns the number of bits.
   * \code{.cpp}
   * size_t size() const noexcept;
   * \endcode
   *
   * \return Number of bits.
   */
  [[nodiscard]] size_t size() const noexcept {
    return m_header == nullptr ? 0 : m_header->size;
  }

  /**
   * \brief Checks whether the bitset is empty.
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
   * \brief Returns the number of storage words.
   * \code{.cpp}
   * size_t word_count() const noexcept;
   * \endcode
   *
   * \return Number of storage words.
   */
  [[nodiscard]] size_t word_count() const noexcept {
    return words_for(size());
  }

  /**
   * \brief Sets, clears, or flips a bit.
   * \code{.cpp}
   * void set(size_t index) noexcept;
   * \endcode
   *
   * \param index Bit index.
   *
   * \attention 1. \p index must be less than size().
   */
  void set(size_t index) noexcept {
    assert(index < size());
    words(m_header)[index / word_bits] |=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  /**
   * \brief Clears a bit.
   * \code{.cpp}
   * void reset(size_t index) noexcept;
   * \endcode
   */
  void reset(size_t index) noexcept {
    assert(index < size());
    words(m_header)[index / word_bits] &=
        static_cast<word_type>(~(word_type{1} << (index % word_bits)));
  }

  /**
   * \brief Flips a bit.
   * \code{.cpp}
   * void flip(size_t index) noexcept;
   * \endcode
   */
  void flip(size_t index) noexcept {
    assert(index < size());
    words(m_header)[index / word_bits] ^=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  /**
   * \brief Returns the value of a bit.
   * \code{.cpp}
   * bool test(size_t index) const noexcept;
   * \endcode
   * \param index Bit index.
   * \return Whether the bit is set.
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]] bool test(size_t index) const noexcept {
    assert(index < size());
    return (words(m_header)[index / word_bits] &
            static_cast<word_type>(word_type{1} << (index % word_bits))) != 0;
  }

  /**
   * \brief Accesses the value of a bit.
   * \code{.cpp}
   * bool operator[](size_t index) const noexcept;
   * \endcode
   * \param index Bit index.
   * \return Whether the bit is set.
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]] bool operator[](size_t index) const noexcept {
    return test(index);
  }

  /**
   * \brief Sets, clears, or flips all bits.
   * \code{.cpp}
   * void set_all() noexcept;
   * \endcode
   */
  void set_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] = full_word;
    }

    if (size() % word_bits != 0) {
      words(m_header)[word_count() - 1] &= last_mask();
    }
  }

  /**
   * \brief Clears all bits.
   * \code{.cpp}
   * void reset_all() noexcept;
   * \endcode
   */
  void reset_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] = 0;
    }
  }

  /**
   * \brief Flips all bits.
   * \code{.cpp}
   * void flip_all() noexcept;
   * \endcode
   */
  void flip_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] =
          static_cast<word_type>(~words(m_header)[i]);
    }

    if (size() % word_bits != 0) {
      words(m_header)[word_count() - 1] &= last_mask();
    }
  }

  /**
   * \brief Checks whether any bit is set.
   * \code{.cpp}
   * bool any() const noexcept;
   * \endcode
   *
   * \return Whether any bit is set.
   */
  [[nodiscard]] bool any() const noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      if (words(m_header)[i] != 0) {
        return true;
      }
    }

    return false;
  }

  /**
   * \brief Checks whether no bits are set.
   * \code{.cpp}
   * bool none() const noexcept;
   * \endcode
   * \return Whether no bit is set.
   */
  [[nodiscard]] bool none() const noexcept {
    return !any();
  }

  /**
   * \brief Checks whether every bit is set.
   * \code{.cpp}
   * bool all() const noexcept;
   * \endcode
   * \return Whether every bit is set.
   */
  [[nodiscard]] bool all() const noexcept {
    const size_t count = word_count();

    if (count == 0) {
      return true;
    }

    for (size_t i = 0; i + 1 < count; ++i) {
      if (words(m_header)[i] != full_word) {
        return false;
      }
    }

    return words(m_header)[count - 1] == last_mask();
  }

  /**
   * \brief Counts the set bits.
   * \code{.cpp}
   * size_t count() const noexcept;
   * \endcode
   * \return Number of set bits.
   */
  [[nodiscard]] size_t count() const noexcept {
    size_t result = 0;

    for (size_t i = 0; i < word_count(); ++i) {
      result += static_cast<size_t>(std::popcount(words(m_header)[i]));
    }

    return result;
  }

  /**
   * \brief Calls a function for each set bit.
   * \code{.cpp}
   * template<typename Fn>
   * void for_each_set_bit(Fn&& fn) const;
   * \endcode
   *
   * \param fn Function receiving each set bit index.
   *
   * Set bits are visited in ascending order.
   */
  template <typename Fn>
  void for_each_set_bit(Fn &&fn) const
      noexcept(noexcept(std::declval<Fn &>()(size_t{}))) {
    for (size_t word_index = 0; word_index < word_count(); ++word_index) {
      word_type value = words(m_header)[word_index];

      while (value != 0) {
        const size_t bit = static_cast<size_t>(std::countr_zero(value));
        fn(word_index * word_bits + bit);
        value &= static_cast<word_type>(value - 1);
      }
    }
  }

  /**
   * \brief Accesses a storage word.
   * \code{.cpp}
   * word_type word(size_t index) const noexcept;
   * \endcode
   *
   * \param index Word index.
   * \param value New word value.
   *
   * \return The requested word.
   *
   * \attention 1. \p index must be less than word_count().
   * \attention 2. Bits outside size() are ignored by set_word().
   */
  [[nodiscard]] word_type word(size_t index) const noexcept {
    assert(index < word_count());
    return words(m_header)[index];
  }

  /**
   * \brief Replaces a storage word.
   * \code{.cpp}
   * void set_word(size_t index, word_type value) noexcept;
   * \endcode
   */
  void set_word(size_t index, word_type value) noexcept {
    assert(index < word_count());

    if (index + 1 == word_count()) {
      value &= last_mask();
    }

    words(m_header)[index] = value;
  }

  /**
   * \brief Returns the underlying word storage.
   * \code{.cpp}
   * const word_type* data() const noexcept;
   * \endcode
   *
   * \return Pointer to the first storage word.
   */
  [[nodiscard]] const word_type *data() const noexcept {
    return words(m_header);
  }

  /**
   * \brief Returns the bitset allocator.
   * \code{.cpp}
   * allocator_type get_allocator() const;
   * \endcode
   *
   * \return A copy of the allocator.
   *
   * \attention 1. The bitset must not be moved-from.
   */
  [[nodiscard]] allocator_type get_allocator() const {
    assert(m_header != nullptr);
    return m_header->allocator;
  }

  /**
   * \brief Applies binary bitwise assignment operations.
   * \code{.cpp}
   * BoxedBitset& operator|=(const BoxedBitset& other) noexcept;
   * \endcode
   *
   * \param other Other bitset.
   *
   * \return This bitset.
   *
   * \attention 1. Both bitsets must have the same size.
   */
  BoxedBitset &operator|=(const BoxedBitset &other) noexcept {
    assert(size() == other.size());

    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] |= other.word(i);
    }

    return *this;
  }

  /**
   * \brief Applies bitwise and assignment.
   * \code{.cpp}
   * BoxedBitset &operator&=(const BoxedBitset &other) noexcept;
   * \endcode
   */
  BoxedBitset &operator&=(const BoxedBitset &other) noexcept {
    assert(size() == other.size());

    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] &= other.word(i);
    }

    return *this;
  }

  /**
   * \brief Applies bitwise xor assignment.
   * \code{.cpp}
   * BoxedBitset &operator^=(const BoxedBitset &other) noexcept;
   * \endcode
   */
  BoxedBitset &operator^=(const BoxedBitset &other) noexcept {
    assert(size() == other.size());

    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] ^= other.word(i);
    }

    return *this;
  }

  /**
   * \brief Clears bits using a mask.
   * \code{.cpp}
   * BoxedBitset &operator-=(const BoxedBitset &other) noexcept;
   * \endcode
   */
  BoxedBitset &operator-=(const BoxedBitset &other) noexcept {
    assert(size() == other.size());

    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] &= static_cast<word_type>(~other.word(i));
    }

    return *this;
  }

  /**
   * \brief Clears bits present in a mask.
   * \code{.cpp}
   * void clear_bits(const BoxedBitset& mask) noexcept;
   * \endcode
   *
   * \param mask Bits to clear.
   *
   * \attention 1. Both bitsets must have the same size.
   */
  void clear_bits(const BoxedBitset &mask) noexcept {
    *this -= mask;
  }

  /**
   * \brief Applies binary bitwise operations.
   * \code{.cpp}
   * BoxedBitset operator|(BoxedBitset lhs, const BoxedBitset& rhs);
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \param value Operand to complement.
   *
   * \return The resulting bitset.
   *
   * \attention 1. Binary operations require equal bitset sizes.
   */
  [[nodiscard]] friend BoxedBitset operator|(
      BoxedBitset lhs, const BoxedBitset &rhs) {
    lhs |= rhs;
    return lhs;
  }

  [[nodiscard]] friend BoxedBitset operator&(
      BoxedBitset lhs, const BoxedBitset &rhs) {
    lhs &= rhs;
    return lhs;
  }

  [[nodiscard]] friend BoxedBitset operator^(
      BoxedBitset lhs, const BoxedBitset &rhs) {
    lhs ^= rhs;
    return lhs;
  }

  [[nodiscard]] friend BoxedBitset operator-(
      BoxedBitset lhs, const BoxedBitset &rhs) {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend BoxedBitset operator~(BoxedBitset value) {
    value.flip_all();
    return value;
  }

  /**
   * \brief Compares two bitsets.
   * \code{.cpp}
   * bool operator==(const BoxedBitset& lhs,
   *                 const BoxedBitset& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   *
   * \return Whether both bitsets have equal sizes and bits.
   */
  [[nodiscard]] friend bool operator==(const BoxedBitset &lhs,
                                       const BoxedBitset &rhs) noexcept {
    if (lhs.size() != rhs.size()) {
      return false;
    }

    for (size_t i = 0; i < lhs.word_count(); ++i) {
      if (lhs.word(i) != rhs.word(i)) {
        return false;
      }
    }

    return true;
  }

private:
  Header *m_header = nullptr;
};

} // namespace strobe

namespace std {

template <typename Word, strobe::Allocator Alloc>
struct hash<strobe::BoxedBitset<Word, Alloc>> {
  size_t operator()(const strobe::BoxedBitset<Word, Alloc> &bits) const
      noexcept {
    size_t result = static_cast<size_t>(1469598103934665603ull);
    result ^= bits.size() + static_cast<size_t>(0x9e3779b97f4a7c15ull) +
              (result << 6) + (result >> 2);
    for (size_t i = 0; i < bits.word_count(); ++i) {
      result ^= static_cast<size_t>(bits.word(i)) +
                static_cast<size_t>(0x9e3779b97f4a7c15ull) +
                (result << 6) + (result >> 2);
    }
    return result;
  }
};

} // namespace std

namespace fmt {

template <typename Word, strobe::Allocator Alloc>
struct formatter<strobe::BoxedBitset<Word, Alloc>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::BoxedBitset<Word, Alloc> &bits,
              FormatContext &ctx) const {
    auto out = ctx.out();
    for (size_t i = bits.size(); i != 0; --i) {
      *out++ = bits.test(i - 1) ? '1' : '0';
    }
    return out;
  }
};

} // namespace fmt
