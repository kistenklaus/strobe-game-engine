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
 * \brief Fixed-capacity, dynamically sized bit vector.
 * \code{.cpp}
 * template<size_t Capacity, typename Word = uint64_t>
 * class InplaceBitVector;
 * \endcode
 *
 * Stores up to \p Capacity bits without dynamic allocation.
 *
 * \attention 1. \p Capacity is the maximum number of bits.
 * \attention 2. \p Word must be an unsigned integer type other than bool.
 * \attention 3. Bits outside the current size are always clear.
 * \attention 4. Bitwise operations require equal vector sizes.
 */
template <size_t Capacity, typename Word = uint64_t>
  requires(std::unsigned_integral<Word> && !std::same_as<Word, bool>)
class InplaceBitVector {
public:
  using word_type = Word;

  /**
   * \brief Maximum number of active bits.
   * \code{.cpp}
   * static constexpr size_t capacity = Capacity;
   * \endcode
   */
  static constexpr size_t capacity = Capacity;

  /**
   * \brief Number of bits in each storage word.
   * \code{.cpp}
   * static constexpr size_t word_bits = std::numeric_limits<word_type>::digits;
   * \endcode
   */
  static constexpr size_t word_bits = std::numeric_limits<word_type>::digits;

  /**
   * \brief Number of words reserved by the vector.
   * \code{.cpp}
   * static constexpr size_t word_capacity = ...;
   * \endcode
   */
  static constexpr size_t word_capacity =
      capacity / word_bits + (capacity % word_bits != 0);

private:
  static constexpr word_type full_word = std::numeric_limits<word_type>::max();

  [[nodiscard]] static constexpr size_t words_for(size_t bits) noexcept {
    return bits / word_bits + (bits % word_bits != 0);
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

  [[nodiscard]] constexpr word_type last_mask() const noexcept {
    const size_t tail = m_size % word_bits;
    return tail == 0 ? full_word : low_mask(tail);
  }

public:
  /**
   * \brief Constructs an empty or sized vector.
   * \code{.cpp}
   * constexpr InplaceBitVector() noexcept;
   * explicit constexpr InplaceBitVector(size_t size) noexcept;
   * \endcode
   *
   * \param size Initial number of bits.
   *
   * All bits are initially clear.
   *
   * \attention 1. \p size must not exceed capacity.
   */
  constexpr InplaceBitVector() noexcept = default;

  explicit constexpr InplaceBitVector(size_t size) noexcept : m_size(size) {
    assert(size <= capacity);
  }

  /**
   * \brief Constructs a full vector.
   * \code{.cpp}
   * static constexpr InplaceBitVector full(size_t size) noexcept;
   * \endcode
   *
   * \param size Number of bits.
   *
   * \return A vector with every bit set.
   *
   * \attention 1. \p size must not exceed capacity.
   */
  [[nodiscard]] static constexpr InplaceBitVector full(size_t size) noexcept {
    InplaceBitVector result(size);
    result.set_all();
    return result;
  }

  /**
   * \brief Returns the number of active bits.
   * \code{.cpp}
   * constexpr size_t size() const noexcept;
   * \endcode
   * \return Number of active bits.
   */
  [[nodiscard]] constexpr size_t size() const noexcept {
    return m_size;
  }

  /**
   * \brief Checks whether the vector is empty.
   * \code{.cpp}
   * constexpr bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]] constexpr bool empty() const noexcept {
    return m_size == 0;
  }

  /**
   * \brief Returns the number of active storage words.
   * \code{.cpp}
   * constexpr size_t word_count() const noexcept;
   * \endcode
   * \return Number of storage words.
   */
  [[nodiscard]] constexpr size_t word_count() const noexcept {
    return words_for(m_size);
  }

  /**
   * \brief Changes the number of active bits.
   * \code{.cpp}
   * constexpr void resize(size_t size) noexcept;
   * \endcode
   *
   * Existing bits are preserved and new bits are clear.
   *
   * \param size New number of bits.
   *
   * \attention 1. \p size must not exceed capacity.
   */
  constexpr void resize(size_t newSize) noexcept {
    assert(newSize <= capacity);

    if (newSize < m_size) {
      const size_t retainedWords = words_for(newSize);

      if (retainedWords != 0) {
        const size_t tail = newSize % word_bits;

        if (tail != 0) {
          m_words[retainedWords - 1] &= low_mask(tail);
        }
      }

      for (size_t i = retainedWords; i < word_count(); ++i) {
        m_words[i] = 0;
      }
    }

    m_size = newSize;
  }

  /**
   * \brief Sets a bit.
   * \code{.cpp}
   * constexpr void set(size_t index) noexcept;
   * \endcode
   *
   * \param index Bit index.
   *
   * \attention 1. \p index must be less than size().
   */
  constexpr void set(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] |=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  /**
   * \brief Clears a bit.
   * \code{.cpp}
   * constexpr void reset(size_t index) noexcept;
   * \endcode
   * \param index Bit index.
   * \attention 1. \p index must be less than size().
   */
  constexpr void reset(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] &=
        static_cast<word_type>(~(word_type{1} << (index % word_bits)));
  }

  /**
   * \brief Flips a bit.
   * \code{.cpp}
   * constexpr void flip(size_t index) noexcept;
   * \endcode
   * \param index Bit index.
   * \attention 1. \p index must be less than size().
   */
  constexpr void flip(size_t index) noexcept {
    assert(index < m_size);
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
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]] constexpr bool test(size_t index) const noexcept {
    assert(index < m_size);
    return (m_words[index / word_bits] &
            static_cast<word_type>(word_type{1} << (index % word_bits))) != 0;
  }

  /**
   * \brief Returns the value of a bit.
   * \code{.cpp}
   * constexpr bool operator[](size_t index) const noexcept;
   * \endcode
   * \param index Bit index.
   * \return Whether the bit is set.
   * \attention 1. \p index must be less than size().
   */
  [[nodiscard]] constexpr bool operator[](size_t index) const noexcept {
    return test(index);
  }

  /**
   * \brief Sets all active bits.
   * \code{.cpp}
   * constexpr void set_all() noexcept;
   * \endcode
   */
  constexpr void set_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = full_word;
    }

    if (m_size % word_bits != 0) {
      m_words[word_count() - 1] &= last_mask();
    }
  }

  /**
   * \brief Clears all active bits.
   * \code{.cpp}
   * constexpr void reset_all() noexcept;
   * \endcode
   */
  constexpr void reset_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = 0;
    }
  }

  /**
   * \brief Flips all active bits.
   * \code{.cpp}
   * constexpr void flip_all() noexcept;
   * \endcode
   */
  constexpr void flip_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = static_cast<word_type>(~m_words[i]);
    }

    if (m_size % word_bits != 0) {
      m_words[word_count() - 1] &= last_mask();
    }
  }

  /**
   * \brief Tests whether any active bit is set.
   * \code{.cpp}
   * constexpr bool any() const noexcept;
   * \endcode
   * \return Whether at least one bit is set.
   */
  [[nodiscard]] constexpr bool any() const noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      if (m_words[i] != 0) {
        return true;
      }
    }

    return false;
  }

  /**
   * \brief Tests whether no active bits are set.
   * \code{.cpp}
   * constexpr bool none() const noexcept;
   * \endcode
   * \return Whether no bit is set.
   */
  [[nodiscard]] constexpr bool none() const noexcept {
    return !any();
  }

  /**
   * \brief Tests whether all active bits are set.
   * \code{.cpp}
   * constexpr bool all() const noexcept;
   * \endcode
   * \return Whether every active bit is set.
   */
  [[nodiscard]] constexpr bool all() const noexcept {
    const size_t count = word_count();

    if (count == 0) {
      return true;
    }

    for (size_t i = 0; i + 1 < count; ++i) {
      if (m_words[i] != full_word) {
        return false;
      }
    }

    return m_words[count - 1] == last_mask();
  }

  /**
   * \brief Counts the set bits.
   * \code{.cpp}
   * constexpr size_t count() const noexcept;
   * \endcode
   * \return Number of set bits.
   */
  [[nodiscard]] constexpr size_t count() const noexcept {
    size_t result = 0;

    for (size_t i = 0; i < word_count(); ++i) {
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
    for (size_t word_index = 0; word_index < word_count(); ++word_index) {
      word_type value = m_words[word_index];

      while (value != 0) {
        const size_t bit = static_cast<size_t>(std::countr_zero(value));
        fn(word_index * word_bits + bit);
        value &= static_cast<word_type>(value - 1);
      }
    }
  }

  /**
   * \brief Returns a storage word.
   * \code{.cpp}
   * constexpr word_type word(size_t index) const noexcept;
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
  [[nodiscard]] constexpr word_type word(size_t index) const noexcept {
    assert(index < word_count());
    return m_words[index];
  }

  /**
   * \brief Replaces a storage word.
   * \code{.cpp}
   * constexpr void set_word(size_t index, word_type value) noexcept;
   * \endcode
   * \param index Word index.
   * \param value New word value.
   * \attention 1. \p index must be less than word_count().
   * \attention 2. Bits outside size() are ignored.
   */
  constexpr void set_word(size_t index, word_type value) noexcept {
    assert(index < word_count());

    if (index + 1 == word_count()) {
      value &= last_mask();
    }

    m_words[index] = value;
  }

  /**
   * \brief Returns the underlying storage.
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
   * \brief Applies bitwise-or assignment.
   * \code{.cpp}
   * constexpr InplaceBitVector& operator|=(
   *     const InplaceBitVector& other) noexcept;
   * \endcode
   *
   * \param other Other bit vector.
   *
   * \return This bit vector.
   *
   * \attention 1. Both vectors must have equal sizes.
   */
  constexpr InplaceBitVector &operator|=(
      const InplaceBitVector &other) noexcept {
    assert(m_size == other.m_size);

    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] |= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Applies bitwise-and assignment.
   * \code{.cpp}
   * constexpr InplaceBitVector& operator&=(const InplaceBitVector& other) noexcept;
   * \endcode
   * \param other Other bit vector.
   * \return This bit vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  constexpr InplaceBitVector &operator&=(
      const InplaceBitVector &other) noexcept {
    assert(m_size == other.m_size);

    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Applies bitwise-xor assignment.
   * \code{.cpp}
   * constexpr InplaceBitVector& operator^=(const InplaceBitVector& other) noexcept;
   * \endcode
   * \param other Other bit vector.
   * \return This bit vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  constexpr InplaceBitVector &operator^=(
      const InplaceBitVector &other) noexcept {
    assert(m_size == other.m_size);

    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] ^= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Clears bits present in another vector.
   * \code{.cpp}
   * constexpr InplaceBitVector& operator-=(const InplaceBitVector& other) noexcept;
   * \endcode
   * \param other Other bit vector.
   * \return This bit vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  constexpr InplaceBitVector &operator-=(
      const InplaceBitVector &other) noexcept {
    assert(m_size == other.m_size);

    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= static_cast<word_type>(~other.m_words[i]);
    }

    return *this;
  }

  /**
   * \brief Clears bits present in a mask.
   * \code{.cpp}
   * constexpr void clear_bits(const InplaceBitVector& mask) noexcept;
   * \endcode
   *
   * \param mask Bits to clear.
   *
   * \attention 1. Both vectors must have equal sizes.
   */
  constexpr void clear_bits(const InplaceBitVector &mask) noexcept {
    *this -= mask;
  }

  /**
   * \brief Computes bitwise or.
   * \code{.cpp}
   * constexpr InplaceBitVector operator|(InplaceBitVector lhs, const InplaceBitVector& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting bit vector.
   *
   * \attention 1. Binary operations require equal vector sizes.
   */
  [[nodiscard]] friend constexpr InplaceBitVector operator|(
      InplaceBitVector lhs, const InplaceBitVector &rhs) noexcept {
    lhs |= rhs;
    return lhs;
  }

  /**
   * \brief Computes bitwise and.
   * \code{.cpp}
   * constexpr InplaceBitVector operator&(InplaceBitVector lhs, const InplaceBitVector& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting bit vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  [[nodiscard]] friend constexpr InplaceBitVector operator&(
      InplaceBitVector lhs, const InplaceBitVector &rhs) noexcept {
    lhs &= rhs;
    return lhs;
  }

  /**
   * \brief Computes bitwise xor.
   * \code{.cpp}
   * constexpr InplaceBitVector operator^(InplaceBitVector lhs, const InplaceBitVector& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting bit vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  [[nodiscard]] friend constexpr InplaceBitVector operator^(
      InplaceBitVector lhs, const InplaceBitVector &rhs) noexcept {
    lhs ^= rhs;
    return lhs;
  }

  /**
   * \brief Clears bits present in another vector.
   * \code{.cpp}
   * constexpr InplaceBitVector operator-(InplaceBitVector lhs, const InplaceBitVector& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting bit vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  [[nodiscard]] friend constexpr InplaceBitVector operator-(
      InplaceBitVector lhs, const InplaceBitVector &rhs) noexcept {
    lhs -= rhs;
    return lhs;
  }

  /**
   * \brief Complements all active bits.
   * \code{.cpp}
   * constexpr InplaceBitVector operator~(InplaceBitVector value) noexcept;
   * \endcode
   * \param value Operand to complement.
   * \return The complemented bit vector.
   */
  [[nodiscard]] friend constexpr InplaceBitVector operator~(
      InplaceBitVector value) noexcept {
    value.flip_all();
    return value;
  }

  /**
   * \brief Compares two bit vectors.
   * \code{.cpp}
   * constexpr bool operator==(const InplaceBitVector& lhs,
   *                            const InplaceBitVector& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   *
   * \return Whether both vectors have equal sizes and bits.
   */
  [[nodiscard]] friend constexpr bool operator==(
      const InplaceBitVector &lhs,
      const InplaceBitVector &rhs) noexcept {
    if (lhs.m_size != rhs.m_size) {
      return false;
    }

    for (size_t i = 0; i < lhs.word_count(); ++i) {
      if (lhs.m_words[i] != rhs.m_words[i]) {
        return false;
      }
    }

    return true;
  }

private:
  std::array<word_type, word_capacity> m_words{};
  size_t m_size = 0;
};

} // namespace strobe

namespace std {

template <size_t Capacity, typename Word>
struct hash<strobe::InplaceBitVector<Capacity, Word>> {
  size_t operator()(
      const strobe::InplaceBitVector<Capacity, Word> &bits) const noexcept {
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

template <size_t Capacity, typename Word>
struct formatter<strobe::InplaceBitVector<Capacity, Word>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::InplaceBitVector<Capacity, Word> &bits,
              FormatContext &ctx) const {
    auto out = ctx.out();
    for (size_t i = bits.size(); i != 0; --i) {
      *out++ = bits.test(i - 1) ? '1' : '0';
    }
    return out;
  }
};

} // namespace fmt
