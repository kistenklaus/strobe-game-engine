#pragma once

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <fmt/format.h>
#include <limits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Fixed-size bitset.
 * \code{.cpp}
 * template<size_t BitCount, typename Word = uint64_t>
 * class Bitset;
 * \endcode
 *
 * Stores \p BitCount bits in unsigned words. Bit zero is the least significant
 * bit of the first word.
 *
 * \attention 1. \p BitCount must be greater than zero.
 * \attention 2. \p Word must be an unsigned integer type other than bool.
 * \attention 3. Unused bits in the final word are always clear.
 */
template <std::size_t BitCount, typename Word = uint64_t>
  requires(std::unsigned_integral<Word> && !std::same_as<Word, bool>)
class Bitset {
public:
  using word_type = Word;

  static_assert(BitCount > 0);

  static constexpr size_t bit_count = BitCount;
  static constexpr size_t word_bits = std::numeric_limits<word_type>::digits;
  static constexpr size_t word_count =
      bit_count / word_bits + (bit_count % word_bits != 0);

private:
  static constexpr word_type full_word = std::numeric_limits<word_type>::max();
  static constexpr size_t tail_bits = bit_count % word_bits;

  static constexpr word_type tail_mask = [] {
    if constexpr (tail_bits == 0) {
      return full_word;
    } else {
      return static_cast<word_type>((word_type{1} << tail_bits) - word_type{1});
    }
  }();

public:
  /**
   * \brief Constructs an empty bitset.
   * \code{.cpp}
   * constexpr Bitset() noexcept;
   * \endcode
   */
  constexpr Bitset() noexcept = default;

  /**
   * \brief Returns a bitset with every bit set.
   * \code{.cpp}
   * static constexpr Bitset full() noexcept;
   * \endcode
   *
   * \return A full bitset.
   */
  [[nodiscard]] static constexpr Bitset full() noexcept {
    Bitset result;
    result.set_all();
    return result;
  }

  /**
   * \brief Sets a bit.
   * \code{.cpp}
   * constexpr void set(size_t index) noexcept;
   * \endcode
   *
   * \param index Bit index.
   *
   * \attention 1. \p index must be less than bit_count.
   */
  constexpr void set(size_t index) noexcept {
    assert(index < bit_count);
    m_words[index / word_bits] |=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  /**
   * \brief Clears a bit.
   * \code{.cpp}
   * constexpr void reset(size_t index) noexcept;
   * \endcode
   *
   * \param index Bit index.
   *
   * \attention 1. \p index must be less than bit_count.
   */
  constexpr void reset(size_t index) noexcept {
    assert(index < bit_count);
    m_words[index / word_bits] &=
        static_cast<word_type>(~(word_type{1} << (index % word_bits)));
  }

  /**
   * \brief Flips a bit.
   * \code{.cpp}
   * constexpr void flip(size_t index) noexcept;
   * \endcode
   *
   * \param index Bit index.
   *
   * \attention 1. \p index must be less than bit_count.
   */
  constexpr void flip(size_t index) noexcept {
    assert(index < bit_count);
    m_words[index / word_bits] ^=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  /**
   * \brief Returns the value of a bit.
   * \code{.cpp}
   * constexpr bool test(size_t index) const noexcept;
   * \endcode
   *
   * \param index Bit index.
   *
   * \return Whether the bit is set.
   *
   * \attention 1. \p index must be less than bit_count.
   */
  [[nodiscard]] constexpr bool test(size_t index) const noexcept {
    assert(index < bit_count);
    return (m_words[index / word_bits] &
            static_cast<word_type>(word_type{1} << (index % word_bits))) != 0;
  }

  /**
   * \brief Accesses the value of a bit.
   * \code{.cpp}
   * constexpr bool operator[](size_t index) const noexcept;
   * \endcode
   *
   * \param index Bit index.
   *
   * \return Whether the bit is set.
   *
   * \attention 1. \p index must be less than bit_count.
   */
  [[nodiscard]] constexpr bool operator[](size_t index) const noexcept {
    return test(index);
  }

  /**
   * \brief Sets all bits.
   * \code{.cpp}
   * constexpr void set_all() noexcept;
   * \endcode
   */
  constexpr void set_all() noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] = full_word;
    }

    if constexpr (tail_bits != 0) {
      m_words[word_count - 1] = tail_mask;
    }
  }

  /**
   * \brief Clears all bits.
   * \code{.cpp}
   * constexpr void reset_all() noexcept;
   * \endcode
   */
  constexpr void reset_all() noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] = 0;
    }
  }

  /**
   * \brief Flips all bits.
   * \code{.cpp}
   * constexpr void flip_all() noexcept;
   * \endcode
   */
  constexpr void flip_all() noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] = static_cast<word_type>(~m_words[i]);
    }

    if constexpr (tail_bits != 0) {
      m_words[word_count - 1] &= tail_mask;
    }
  }

  /**
   * \brief Checks whether any bit is set.
   * \code{.cpp}
   * constexpr bool any() const noexcept;
   * \endcode
   *
   * \return Whether any bit is set.
   */
  [[nodiscard]] constexpr bool any() const noexcept {
    word_type merged = 0;

    for (size_t i = 0; i < word_count; ++i) {
      merged |= m_words[i];
    }

    return merged != 0;
  }

  /**
   * \brief Checks whether no bits are set.
   * \code{.cpp}
   * constexpr bool none() const noexcept;
   * \endcode
   *
   * \return Whether no bits are set.
   */
  [[nodiscard]] constexpr bool none() const noexcept {
    return !any();
  }

  /**
   * \brief Checks whether every bit is set.
   * \code{.cpp}
   * constexpr bool all() const noexcept;
   * \endcode
   *
   * \return Whether every bit is set.
   */
  [[nodiscard]] constexpr bool all() const noexcept {
    word_type missing = 0;

    for (size_t i = 0; i + 1 < word_count; ++i) {
      missing |= static_cast<word_type>(~m_words[i]);
    }

    missing |= static_cast<word_type>(m_words[word_count - 1] ^ tail_mask);

    return missing == 0;
  }

  /**
   * \brief Counts the set bits.
   * \code{.cpp}
   * constexpr size_t count() const noexcept;
   * \endcode
   *
   * \return Number of set bits.
   */
  [[nodiscard]] constexpr size_t count() const noexcept {
    size_t result = 0;

    for (size_t i = 0; i < word_count; ++i) {
      result += static_cast<size_t>(std::popcount(m_words[i]));
    }

    return result;
  }

  /**
   * \brief Calls a function for each set bit.
   * \code{.cpp}
   * template<typename Fn>
   * constexpr void for_each_set_bit(Fn&& fn) const;
   * \endcode
   *
   * \param fn Function receiving each set bit index.
   *
   * Set bits are visited in ascending order.
   */
  template <typename Fn>
  constexpr void for_each_set_bit(Fn &&fn) const
      noexcept(noexcept(std::declval<Fn &>()(size_t{}))) {
    for (size_t word_index = 0; word_index < word_count; ++word_index) {
      word_type word = m_words[word_index];

      while (word != 0) {
        const size_t bit = static_cast<size_t>(std::countr_zero(word));
        fn(word_index * word_bits + bit);
        word &= static_cast<word_type>(word - 1);
      }
    }
  }

  /**
   * \brief Accesses a storage word.
   * \code{.cpp}
   * constexpr word_type word(size_t index) const noexcept;
   * \endcode
   *
   * \param index Word index.
   *
   * \return The requested word.
   *
   * \attention 1. \p index must be less than word_count.
   */
  [[nodiscard]] constexpr word_type word(size_t index) const noexcept {
    assert(index < word_count);
    return m_words[index];
  }

  /**
   * \brief Replaces a storage word.
   * \code{.cpp}
   * constexpr void set_word(size_t index, word_type value) noexcept;
   * \endcode
   *
   * \param index Word index.
   * \param value New word value.
   *
   * \attention 1. \p index must be less than word_count.
   * \attention 2. Bits outside bit_count are ignored.
   */
  constexpr void set_word(size_t index, word_type value) noexcept {
    assert(index < word_count);

    if constexpr (tail_bits != 0) {
      if (index + 1 == word_count) {
        value &= tail_mask;
      }
    }

    m_words[index] = value;
  }

  /**
   * \brief Returns the underlying word storage.
   * \code{.cpp}
   * constexpr const word_type* data() const noexcept;
   * \endcode
   *
   * \return Pointer to the first storage word.
   */
  [[nodiscard]] constexpr const word_type *data() const noexcept {
    return m_words.data();
  }

  /**
   * \brief Applies bitwise union.
   * \code{.cpp}
   * constexpr Bitset& operator|=(const Bitset& other) noexcept;
   * \endcode
   *
   * \param other Other bitset.
   *
   * \return This bitset.
   */
  constexpr Bitset &operator|=(const Bitset &other) noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] |= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Applies bitwise intersection.
   * \code{.cpp}
   * constexpr Bitset& operator&=(const Bitset& other) noexcept;
   * \endcode
   *
   * \param other Other bitset.
   * \return This bitset.
   */
  constexpr Bitset &operator&=(const Bitset &other) noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] &= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Applies bitwise symmetric difference.
   * \code{.cpp}
   * constexpr Bitset& operator^=(const Bitset& other) noexcept;
   * \endcode
   *
   * \param other Other bitset.
   * \return This bitset.
   */
  constexpr Bitset &operator^=(const Bitset &other) noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] ^= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Removes bits present in another bitset.
   * \code{.cpp}
   * constexpr Bitset& operator-=(const Bitset& other) noexcept;
   * \endcode
   *
   * \param other Other bitset.
   * \return This bitset.
   */
  constexpr Bitset &operator-=(const Bitset &other) noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] &= static_cast<word_type>(~other.m_words[i]);
    }

    return *this;
  }

  /**
   * \brief Clears bits present in a mask.
   * \code{.cpp}
   * constexpr void clear_bits(const Bitset& mask) noexcept;
   * \endcode
   *
   * \param mask Bits to clear.
   */
  constexpr void clear_bits(const Bitset &mask) noexcept {
    *this -= mask;
  }

  /**
   * \brief Returns the bitwise union of two bitsets.
   * \code{.cpp}
   * constexpr Bitset operator|(Bitset lhs, const Bitset& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting bitset.
   */
  [[nodiscard]] friend constexpr Bitset
  operator|(Bitset lhs, const Bitset &rhs) noexcept {
    lhs |= rhs;
    return lhs;
  }

  /**
   * \brief Returns the bitwise intersection of two bitsets.
   * \code{.cpp}
   * constexpr Bitset operator&(Bitset lhs, const Bitset& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting bitset.
   */
  [[nodiscard]] friend constexpr Bitset
  operator&(Bitset lhs, const Bitset &rhs) noexcept {
    lhs &= rhs;
    return lhs;
  }

  /**
   * \brief Returns the bitwise symmetric difference of two bitsets.
   * \code{.cpp}
   * constexpr Bitset operator^(Bitset lhs, const Bitset& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting bitset.
   */
  [[nodiscard]] friend constexpr Bitset
  operator^(Bitset lhs, const Bitset &rhs) noexcept {
    lhs ^= rhs;
    return lhs;
  }

  /**
   * \brief Returns the difference of two bitsets.
   * \code{.cpp}
   * constexpr Bitset operator-(Bitset lhs, const Bitset& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting bitset.
   */
  [[nodiscard]] friend constexpr Bitset
  operator-(Bitset lhs, const Bitset &rhs) noexcept {
    lhs -= rhs;
    return lhs;
  }

  /**
   * \brief Returns the complement of a bitset.
   * \code{.cpp}
   * constexpr Bitset operator~(Bitset value) noexcept;
   * \endcode
   *
   * \param value Operand to complement.
   * \return The resulting bitset.
   */
  [[nodiscard]] friend constexpr Bitset
  operator~(Bitset value) noexcept {
    value.flip_all();
    return value;
  }

  /**
   * \brief Compares two bitsets.
   * \code{.cpp}
   * constexpr bool operator==(const Bitset& lhs, const Bitset& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   *
   * \return Whether all bits are equal.
   */
  [[nodiscard]] friend constexpr bool
  operator==(const Bitset &lhs, const Bitset &rhs) noexcept {
    word_type diff = 0;

    for (size_t i = 0; i < word_count; ++i) {
      diff |= static_cast<word_type>(lhs.m_words[i] ^ rhs.m_words[i]);
    }

    return diff == 0;
  }

private:
  std::array<word_type, word_count> m_words{};
};

} // namespace strobe

namespace std {

template <size_t BitCount, typename Word>
struct hash<strobe::Bitset<BitCount, Word>> {
  size_t operator()(const strobe::Bitset<BitCount, Word> &bits) const
      noexcept {
    size_t result = static_cast<size_t>(1469598103934665603ull);
    for (size_t i = 0; i < strobe::Bitset<BitCount, Word>::word_count; ++i) {
      result ^= static_cast<size_t>(bits.word(i)) +
                static_cast<size_t>(0x9e3779b97f4a7c15ull) +
                (result << 6) + (result >> 2);
    }
    return result;
  }
};

} // namespace std

namespace fmt {

template <size_t BitCount, typename Word>
struct formatter<strobe::Bitset<BitCount, Word>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::Bitset<BitCount, Word> &bits,
              FormatContext &ctx) const {
    auto out = ctx.out();
    for (size_t i = BitCount; i != 0; --i) {
      *out++ = bits.test(i - 1) ? '1' : '0';
    }
    return out;
  }
};

} // namespace fmt
