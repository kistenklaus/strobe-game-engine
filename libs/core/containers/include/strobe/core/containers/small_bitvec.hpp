#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <fmt/format.h>
#include <limits>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Bit vector with inline storage and allocator-backed growth.
 * \code{.cpp}
 * template<size_t MinInlineBits = 128, typename Word = uint64_t,
 *          Allocator A = Mallocator>
 * class SmallBitVector;
 * \endcode
 *
 * Stores up to inline_capacity() bits without allocation and grows through
 * the supplied allocator.
 *
 * \attention 1. Operations combining two vectors require equal sizes.
 * \attention 2. Growth may invalidate pointers returned by data().
 */
template <size_t MinInlineBits = 128, typename Word = uint64_t,
          Allocator A = Mallocator>
  requires(std::unsigned_integral<Word> && !std::same_as<Word, bool>)
class SmallBitVector {
public:
  using word_type = Word;

  static constexpr size_t word_bits = std::numeric_limits<Word>::digits;
  static constexpr size_t inline_words =
      MinInlineBits / word_bits + (MinInlineBits % word_bits != 0);

private:
  static constexpr Word full_word = std::numeric_limits<Word>::max();

  [[nodiscard]] static constexpr size_t words_for(size_t bits) noexcept {
    return bits / word_bits + (bits % word_bits != 0);
  }

  [[nodiscard]] static constexpr Word low_mask(size_t bits) noexcept {
    assert(bits <= word_bits);
    if (bits == word_bits) {
      return full_word;
    }
    if (bits == 0) {
      return 0;
    }
    return static_cast<Word>((Word{1} << bits) - Word{1});
  }

  [[nodiscard]] Word last_mask() const noexcept {
    const size_t tail = m_size % word_bits;
    return tail == 0 ? full_word : low_mask(tail);
  }

public:
  /**
   * \brief Constructs, copies, moves, or assigns a bit vector.
   * \code{.cpp}
   * explicit SmallBitVector(const A& alloc = {});
   * explicit SmallBitVector(size_t size, const A& alloc = {});
   * SmallBitVector(const SmallBitVector& other) = default;
   * SmallBitVector& operator=(const SmallBitVector& other) = default;
   * SmallBitVector(SmallBitVector&& other);
   * SmallBitVector& operator=(SmallBitVector&& other);
   * \endcode
   *
   * \param size Initial number of cleared bits.
   * \param alloc Allocator to use.
   * \param other Vector to copy or move from.
   *
   * Moved-from vectors are empty.
   */
  explicit SmallBitVector(const A &alloc = {}) : m_words(alloc) {}

  explicit SmallBitVector(size_t size, const A &alloc = {})
      : m_words(alloc) {
    resize(size);
  }

  /**
   * \brief Returns a vector with every bit set.
   * \code{.cpp}
   * static SmallBitVector full(size_t size, const A& alloc = {});
   * \endcode
   *
   * \param size Number of bits.
   * \param alloc Allocator to use.
   * \return A full bit vector.
   */
  [[nodiscard]] static SmallBitVector full(
      size_t size, const A &alloc = {}) {
    SmallBitVector result(size, alloc);
    result.set_all();
    return result;
  }

  SmallBitVector(const SmallBitVector &) = default;
  SmallBitVector &operator=(const SmallBitVector &) = default;

  SmallBitVector(SmallBitVector &&other) noexcept(
      std::is_nothrow_move_constructible_v<decltype(m_words)>)
      : m_words(std::move(other.m_words)),
        m_size(std::exchange(other.m_size, 0)) {}

  SmallBitVector &operator=(SmallBitVector &&other) noexcept(
      std::is_nothrow_move_assignable_v<decltype(m_words)>) {
    if (this != &other) {
      m_words = std::move(other.m_words);
      m_size = std::exchange(other.m_size, 0);
    }
    return *this;
  }

  /**
   * \brief Returns the number of bits.
   * \code{.cpp}
   * size_t size() const noexcept;
   * \endcode
   * \return Number of bits.
   */
  [[nodiscard]] size_t size() const noexcept { return m_size; }

  /**
   * \brief Checks whether the vector is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

  /**
   * \brief Returns the number of storage words.
   * \code{.cpp}
   * size_t word_count() const noexcept;
   * \endcode
   * \return Number of storage words.
   */
  [[nodiscard]] size_t word_count() const noexcept { return m_words.size(); }

  /**
   * \brief Returns the bit capacity.
   * \code{.cpp}
   * size_t capacity() const noexcept;
   * \endcode
   * \return Capacity measured in bits and rounded to whole storage words.
   */
  [[nodiscard]] size_t capacity() const noexcept {
    return m_words.capacity() * word_bits;
  }

  /**
   * \brief Returns the inline bit capacity.
   * \code{.cpp}
   * static constexpr size_t inline_capacity() noexcept;
   * \endcode
   * \return Capacity available without allocation.
   */
  [[nodiscard]] static constexpr size_t inline_capacity() noexcept {
    return inline_words * word_bits;
  }

  /**
   * \brief Checks whether the vector uses inline storage.
   * \code{.cpp}
   * bool using_inline_storage() const noexcept;
   * \endcode
   * \return Whether the storage resides inside the vector.
   */
  [[nodiscard]] bool using_inline_storage() const noexcept {
    return m_words.using_inline_storage();
  }

  /**
   * \brief Reserves capacity for bits.
   * \code{.cpp}
   * void reserve(size_t bits);
   * \endcode
   * \param bits Desired bit capacity.
   */
  void reserve(size_t bits) {
    m_words.reserve(words_for(bits));
  }

  /**
   * \brief Reduces storage capacity to fit the current size.
   * \code{.cpp}
   * void shrink_to_fit();
   * \endcode
   */
  void shrink_to_fit() {
    m_words.shrink_to_fit();
  }

  /**
   * \brief Changes the number of bits.
   * \code{.cpp}
   * void resize(size_t newSize);
   * \endcode
   * \param newSize New number of bits.
   *
   * Newly added bits are cleared.
   */
  void resize(size_t newSize) {
    const size_t newWords = words_for(newSize);

    if (newSize < m_size && newWords != 0) {
      const size_t tail = newSize % word_bits;
      if (tail != 0) {
        m_words[newWords - 1] &= low_mask(tail);
      }
    }

    // SmallVector value-initializes any newly added words to zero.
    m_words.resize(newWords);
    m_size = newSize;
  }

  /**
   * \brief Sets a bit.
   * \code{.cpp}
   * void set(size_t index) noexcept;
   * \endcode
   * \param index Bit index.
   * \attention 1. \p index must be less than size().
   */
  void set(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] |=
        static_cast<Word>(Word{1} << (index % word_bits));
  }

  /**
   * \brief Clears a bit.
   * \code{.cpp}
   * void reset(size_t index) noexcept;
   * \endcode
   * \param index Bit index.
   * \attention 1. \p index must be less than size().
   */
  void reset(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] &=
        static_cast<Word>(~(Word{1} << (index % word_bits)));
  }

  /**
   * \brief Flips a bit.
   * \code{.cpp}
   * void flip(size_t index) noexcept;
   * \endcode
   * \param index Bit index.
   * \attention 1. \p index must be less than size().
   */
  void flip(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] ^=
        static_cast<Word>(Word{1} << (index % word_bits));
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
    assert(index < m_size);
    return (m_words[index / word_bits] &
            static_cast<Word>(Word{1} << (index % word_bits))) != 0;
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
   * \brief Sets all bits.
   * \code{.cpp}
   * void set_all() noexcept;
   * \endcode
   */
  void set_all() noexcept {
    for (Word &word : m_words) {
      word = full_word;
    }
    if (m_size % word_bits != 0) {
      m_words.back() &= last_mask();
    }
  }

  /**
   * \brief Clears all bits.
   * \code{.cpp}
   * void reset_all() noexcept;
   * \endcode
   */
  void reset_all() noexcept {
    for (Word &word : m_words) {
      word = 0;
    }
  }

  /**
   * \brief Flips all bits.
   * \code{.cpp}
   * void flip_all() noexcept;
   * \endcode
   */
  void flip_all() noexcept {
    for (Word &word : m_words) {
      word = static_cast<Word>(~word);
    }
    if (m_size % word_bits != 0) {
      m_words.back() &= last_mask();
    }
  }

  /**
   * \brief Checks whether any bit is set.
   * \code{.cpp}
   * bool any() const noexcept;
   * \endcode
   * \return Whether any bit is set.
   */
  [[nodiscard]] bool any() const noexcept {
    for (Word word : m_words) {
      if (word != 0) {
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
   * \return Whether no bits are set.
   */
  [[nodiscard]] bool none() const noexcept { return !any(); }

  /**
   * \brief Checks whether every bit is set.
   * \code{.cpp}
   * bool all() const noexcept;
   * \endcode
   * \return Whether every bit is set.
   */
  [[nodiscard]] bool all() const noexcept {
    if (m_words.empty()) {
      return true;
    }
    for (size_t i = 0; i + 1 < word_count(); ++i) {
      if (m_words[i] != full_word) {
        return false;
      }
    }
    return m_words.back() == last_mask();
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
    for (Word word : m_words) {
      result += static_cast<size_t>(std::popcount(word));
    }
    return result;
  }

  /**
   * \brief Calls a function for each set bit.
   * \code{.cpp}
   * template<typename Fn>
   * void for_each_set_bit(Fn&& fn) const;
   * \endcode
   * \param fn Function receiving each set bit index.
   *
   * Set bits are visited in ascending order.
   */
  template <typename Fn>
  void for_each_set_bit(Fn &&fn) const
      noexcept(noexcept(std::declval<Fn &>()(size_t{}))) {
    for (size_t wordIdx = 0; wordIdx < word_count(); ++wordIdx) {
      Word value = m_words[wordIdx];
      while (value != 0) {
        const size_t bit = static_cast<size_t>(std::countr_zero(value));
        fn(wordIdx * word_bits + bit);
        value &= static_cast<Word>(value - 1);
      }
    }
  }

  /**
   * \brief Accesses a storage word.
   * \code{.cpp}
   * word_type word(size_t index) const noexcept;
   * \endcode
   * \param index Word index.
   * \return The requested word.
   * \attention 1. \p index must be less than word_count().
   */
  [[nodiscard]] Word word(size_t index) const noexcept {
    assert(index < word_count());
    return m_words[index];
  }

  /**
   * \brief Replaces a storage word.
   * \code{.cpp}
   * void set_word(size_t index, word_type value) noexcept;
   * \endcode
   * \param index Word index.
   * \param value New word value.
   * \attention 1. \p index must be less than word_count().
   * \attention 2. Bits outside size() are ignored.
   */
  void set_word(size_t index, Word value) noexcept {
    assert(index < word_count());
    if (index + 1 == word_count()) {
      value &= last_mask();
    }
    m_words[index] = value;
  }

  /**
   * \brief Returns the underlying word storage.
   * \code{.cpp}
   * const word_type* data() const noexcept;
   * \endcode
   * \return Pointer to the first storage word.
   */
  [[nodiscard]] const Word *data() const noexcept {
    return m_words.data();
  }

  /**
   * \brief Applies bitwise union.
   * \code{.cpp}
   * SmallBitVector& operator|=(const SmallBitVector& other) noexcept;
   * \endcode
   * \param other Other vector.
   * \return This vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  SmallBitVector &operator|=(const SmallBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] |= other.m_words[i];
    }
    return *this;
  }

  /**
   * \brief Applies bitwise intersection.
   * \code{.cpp}
   * SmallBitVector& operator&=(const SmallBitVector& other) noexcept;
   * \endcode
   * \param other Other vector.
   * \return This vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  SmallBitVector &operator&=(const SmallBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= other.m_words[i];
    }
    return *this;
  }

  /**
   * \brief Applies bitwise symmetric difference.
   * \code{.cpp}
   * SmallBitVector& operator^=(const SmallBitVector& other) noexcept;
   * \endcode
   * \param other Other vector.
   * \return This vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  SmallBitVector &operator^=(const SmallBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] ^= other.m_words[i];
    }
    return *this;
  }

  /**
   * \brief Removes bits present in another vector.
   * \code{.cpp}
   * SmallBitVector& operator-=(const SmallBitVector& other) noexcept;
   * \endcode
   * \param other Other vector.
   * \return This vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  SmallBitVector &operator-=(const SmallBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= static_cast<Word>(~other.m_words[i]);
    }
    return *this;
  }

  /**
   * \brief Clears bits present in a mask.
   * \code{.cpp}
   * void clear_bits(const SmallBitVector& mask) noexcept;
   * \endcode
   * \param mask Bits to clear.
   * \attention 1. Both vectors must have equal sizes.
   */
  void clear_bits(const SmallBitVector &mask) noexcept {
    *this -= mask;
  }

  /**
   * \brief Returns the bitwise union of two vectors.
   * \code{.cpp}
   * SmallBitVector operator|(SmallBitVector lhs,
   *                          const SmallBitVector& rhs);
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  [[nodiscard]] friend SmallBitVector
  operator|(SmallBitVector lhs, const SmallBitVector &rhs) {
    lhs |= rhs;
    return lhs;
  }

  /**
   * \brief Returns the bitwise intersection of two vectors.
   * \code{.cpp}
   * SmallBitVector operator&(SmallBitVector lhs,
   *                          const SmallBitVector& rhs);
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  [[nodiscard]] friend SmallBitVector
  operator&(SmallBitVector lhs, const SmallBitVector &rhs) {
    lhs &= rhs;
    return lhs;
  }

  /**
   * \brief Returns the bitwise symmetric difference of two vectors.
   * \code{.cpp}
   * SmallBitVector operator^(SmallBitVector lhs,
   *                          const SmallBitVector& rhs);
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  [[nodiscard]] friend SmallBitVector
  operator^(SmallBitVector lhs, const SmallBitVector &rhs) {
    lhs ^= rhs;
    return lhs;
  }

  /**
   * \brief Returns the difference of two vectors.
   * \code{.cpp}
   * SmallBitVector operator-(SmallBitVector lhs,
   *                          const SmallBitVector& rhs);
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return The resulting vector.
   * \attention 1. Both vectors must have equal sizes.
   */
  [[nodiscard]] friend SmallBitVector
  operator-(SmallBitVector lhs, const SmallBitVector &rhs) {
    lhs -= rhs;
    return lhs;
  }

  /**
   * \brief Returns the complement of a vector.
   * \code{.cpp}
   * SmallBitVector operator~(SmallBitVector value);
   * \endcode
   * \param value Operand to complement.
   * \return The resulting vector.
   */
  [[nodiscard]] friend SmallBitVector operator~(SmallBitVector value) {
    value.flip_all();
    return value;
  }

  /**
   * \brief Compares two bit vectors.
   * \code{.cpp}
   * bool operator==(const SmallBitVector& lhs,
   *                 const SmallBitVector& rhs) noexcept;
   * \endcode
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \return Whether sizes and bits are equal.
   */
  [[nodiscard]] friend bool
  operator==(const SmallBitVector &lhs,
             const SmallBitVector &rhs) noexcept {
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
  SmallVector<Word, inline_words, A> m_words;
  size_t m_size = 0;
};

} // namespace strobe

namespace std {

/**
 * \brief Hashes a small bit vector by its size and storage words.
 * \code{.cpp}
 * template<size_t MinInlineBits, typename Word, strobe::Allocator A>
 * struct hash<strobe::SmallBitVector<MinInlineBits, Word, A>>;
 * \endcode
 */
template <size_t MinInlineBits, typename Word, strobe::Allocator A>
struct hash<strobe::SmallBitVector<MinInlineBits, Word, A>> {
  size_t operator()(
      const strobe::SmallBitVector<MinInlineBits, Word, A> &bits) const
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

/**
 * \brief Formats a small bit vector as a high-to-low binary sequence.
 * \code{.cpp}
 * template<size_t MinInlineBits, typename Word, strobe::Allocator A>
 * struct formatter<strobe::SmallBitVector<MinInlineBits, Word, A>>;
 * \endcode
 */
template <size_t MinInlineBits, typename Word, strobe::Allocator A>
struct formatter<strobe::SmallBitVector<MinInlineBits, Word, A>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::SmallBitVector<MinInlineBits, Word, A> &bits,
              FormatContext &ctx) const {
    auto out = ctx.out();
    for (size_t i = bits.size(); i != 0; --i) {
      *out++ = bits.test(i - 1) ? '1' : '0';
    }
    return out;
  }
};

} // namespace fmt
