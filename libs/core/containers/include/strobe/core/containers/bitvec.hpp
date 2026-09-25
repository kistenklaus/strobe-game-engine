#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <fmt/format.h>
#include <limits>
#include <type_traits>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Dynamically sized bit vector.
 * \code{.cpp}
 * template<typename Word = uint64_t, Allocator Alloc = Mallocator>
 * class BitVector;
 * \endcode
 *
 * Stores bits in dynamically allocated unsigned words.
 *
 * \attention 1. \p Word must be an unsigned integer type other than bool.
 * \attention 2. Bits outside the current size are always clear.
 * \attention 3. Bit indices must be less than size().
 * \attention 4. Bitwise operations require equal vector sizes.
 */
template <typename Word = uint64_t, Allocator Alloc = Mallocator>
  requires(std::unsigned_integral<Word> && !std::same_as<Word, bool>)
class BitVector {
public:
  using word_type = Word;
  using allocator_type = std::remove_cvref_t<Alloc>;

  static constexpr size_t word_bits = std::numeric_limits<word_type>::digits;

private:
  using allocator_traits = AllocatorTraits<allocator_type>;

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

  [[nodiscard]] word_type *allocate(size_t count) {
    if (count == 0) {
      return nullptr;
    }

    word_type *words =
        allocator_traits::template allocate<word_type>(m_allocator, count);
    assert(words != nullptr);
    return words;
  }

  void release() noexcept {
    if (m_words != nullptr) {
      allocator_traits::template deallocate<word_type>(m_allocator, m_words,
                                                       word_count());
    }

    m_words = nullptr;
    m_size = 0;
  }

public:
  /**
   * \brief Constructs, copies, moves, assigns, or destroys a bit vector.
   * \code{.cpp}
   * BitVector() noexcept;
   * explicit BitVector(size_t size, const allocator_type& alloc = {});
   * BitVector(const BitVector& other);
   * BitVector(BitVector&& other) noexcept(...);
   * BitVector& operator=(const BitVector& other);
   * BitVector& operator=(BitVector&& other) noexcept(...);
   * ~BitVector() noexcept;
   * \endcode
   * \param size Number of bits.
   * \param alloc Allocator to use.
   * \param other Vector to copy or move from.
   * Moved-from vectors are empty.
   */
  BitVector() noexcept = default;

  explicit BitVector(size_t size, const allocator_type &alloc = {})
      : m_allocator(alloc), m_size(size), m_words(allocate(words_for(size))) {
    if (m_words != nullptr) {
      std::memset(m_words, 0, word_count() * sizeof(word_type));
    }
  }

  /**
   * \brief Constructs a vector with every bit set.
   * \code{.cpp}
   * static BitVector full(size_t size, const allocator_type& alloc = {});
   * \endcode
   *
   * \param size Number of bits.
   * \param alloc Allocator to use.
   *
   * \return A full bit vector.
   */
  [[nodiscard]] static BitVector full(size_t size,
                                      const allocator_type &alloc = {}) {
    BitVector result(size, alloc);
    result.set_all();
    return result;
  }

  BitVector(const BitVector &other)
      : m_allocator(other.m_allocator), m_size(other.m_size),
        m_words(allocate(other.word_count())) {
    if (m_words != nullptr) {
      std::memcpy(m_words, other.m_words, word_count() * sizeof(word_type));
    }
  }

  BitVector(BitVector &&other) noexcept(
      std::is_nothrow_move_constructible_v<allocator_type>)
      : m_allocator(std::move(other.m_allocator)),
        m_size(std::exchange(other.m_size, 0)),
        m_words(std::exchange(other.m_words, nullptr)) {}

  BitVector &operator=(const BitVector &other)
    requires std::is_copy_assignable_v<allocator_type>
  {
    if (this == &other) {
      return *this;
    }

    release();
    m_allocator = other.m_allocator;
    m_size = other.m_size;
    m_words = allocate(word_count());

    if (m_words != nullptr) {
      std::memcpy(m_words, other.m_words, word_count() * sizeof(word_type));
    }

    return *this;
  }

  BitVector &operator=(BitVector &&other) noexcept(
      std::is_nothrow_move_assignable_v<allocator_type>)
    requires std::is_move_assignable_v<allocator_type>
  {
    if (this == &other) {
      return *this;
    }

    release();
    m_allocator = std::move(other.m_allocator);
    m_size = std::exchange(other.m_size, 0);
    m_words = std::exchange(other.m_words, nullptr);

    return *this;
  }

  ~BitVector() noexcept { release(); }

  /**
   * \brief Returns the number of bits.
   * \code{.cpp}
   * size_t size() const noexcept;
   * \endcode
   *
   * \return Number of bits.
   */
  [[nodiscard]] size_t size() const noexcept { return m_size; }

  /**
   * \brief Checks whether the vector is empty.
   * \code{.cpp}
   * bool empty() const noexcept;
   * \endcode
   *
   * \return Whether size() is zero.
   */
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

  /**
   * \brief Returns the number of storage words.
   * \code{.cpp}
   * size_t word_count() const noexcept;
   * \endcode
   *
   * \return Number of storage words.
   */
  [[nodiscard]] size_t word_count() const noexcept {
    return words_for(m_size);
  }

  /**
   * \brief Changes the number of bits.
   * \code{.cpp}
   * void resize(size_t size);
   * \endcode
   *
   * New bits are clear. Existing bits are preserved.
   *
   * \param size New number of bits.
   */
  void resize(size_t newSize) {
    if (newSize == m_size) {
      return;
    }

    const size_t oldCount = word_count();
    const size_t newCount = words_for(newSize);

    if (newCount != oldCount) {
      word_type *newWords = allocate(newCount);

      if (newWords != nullptr) {
        std::memset(newWords, 0, newCount * sizeof(word_type));

        if (m_words != nullptr) {
          std::memcpy(newWords, m_words,
                      (newCount < oldCount ? newCount : oldCount) *
                          sizeof(word_type));
        }
      }

      if (m_words != nullptr) {
        allocator_traits::template deallocate<word_type>(m_allocator, m_words,
                                                         oldCount);
      }

      m_words = newWords;
    }

    m_size = newSize;

    if (newCount != 0 && newSize % word_bits != 0) {
      m_words[newCount - 1] &= last_mask();
    }
  }

  /**
   * \brief Sets a bit.
   * \code{.cpp}
   * void set(size_t index) noexcept;
   * \endcode
   *
   * \param index Bit index.
   *
   * \attention 1. \p index must be less than size().
   */
  void set(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] |=
        static_cast<word_type>(word_type{1} << (index % word_bits));
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
        static_cast<word_type>(~(word_type{1} << (index % word_bits)));
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
    assert(index < m_size);
    return (m_words[index / word_bits] &
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
   * \brief Sets all bits.
   * \code{.cpp}
   * void set_all() noexcept;
     * \endcode
   */
  void set_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = full_word;
    }

    if (m_size % word_bits != 0) {
      m_words[word_count() - 1] &= last_mask();
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
      m_words[i] = 0;
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
      m_words[i] = static_cast<word_type>(~m_words[i]);
    }

    if (m_size % word_bits != 0) {
      m_words[word_count() - 1] &= last_mask();
    }
  }

  /**
   * \brief Tests whether any bit is set.
   * \code{.cpp}
   * bool any() const noexcept;
   * \endcode
   * \return Whether at least one bit is set.
   */
  [[nodiscard]] bool any() const noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      if (m_words[i] != 0) {
        return true;
      }
    }

    return false;
  }

  /**
   * \brief Tests whether no bits are set.
   * \code{.cpp}
   * [[nodiscard]] bool none() const noexcept;
   * \endcode
   * \return Whether no bit is set.
   */
  [[nodiscard]] bool none() const noexcept {
    return !any();
  }

  /**
   * \brief Tests whether all bits are set.
   * \code{.cpp}
   * [[nodiscard]] bool all() const noexcept;
   * \endcode
   * \return Whether every bit is set.
   */
  [[nodiscard]] bool all() const noexcept {
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
   * [[nodiscard]] size_t count() const noexcept;
   * \endcode
   * \return The number of set bits.
   */
  [[nodiscard]] size_t count() const noexcept {
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
  void set_word(size_t index, word_type value) noexcept {
    assert(index < word_count());

    if (index + 1 == word_count()) {
      value &= last_mask();
    }

    m_words[index] = value;
  }

  /**
   * \brief Returns the underlying storage.
   * \code{.cpp}
   * const word_type* data() const noexcept;
   * \endcode
   *
   * \return Pointer to the first storage word.
   */
  [[nodiscard]] const word_type *data() const noexcept {
    return m_words;
  }

  /**
   * \brief Applies binary bitwise assignment operations.
   * \code{.cpp}
   * BitVector& operator|=(const BitVector& other) noexcept;
   * \endcode
   *
   * \param other Other bit vector.
   *
   * \return This bit vector.
   *
   * \attention 1. Both vectors must have the same size.
   */
  BitVector &operator|=(const BitVector &other) noexcept {
    assert(m_size == other.m_size);

    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] |= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Applies bitwise and assignment.
   * \code{.cpp}
   * BitVector &operator&=(...);
   * \endcode
   * \param other Other bit vector.
   * \attention 1. Both vectors must have the same size.
   */
  BitVector &operator&=(const BitVector &other) noexcept {
    assert(m_size == other.m_size);

    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Applies bitwise xor assignment.
   * \code{.cpp}
   * BitVector &operator^=(...);
   * \endcode
   * \param other Other bit vector.
   * \attention 1. Both vectors must have the same size.
   */
  BitVector &operator^=(const BitVector &other) noexcept {
    assert(m_size == other.m_size);

    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] ^= other.m_words[i];
    }

    return *this;
  }

  /**
   * \brief Clears bits using a mask.
   * \code{.cpp}
   * BitVector &operator-=(...);
   * \endcode
   * \param other Other bit vector.
   * \attention 1. Both vectors must have the same size.
   */
  BitVector &operator-=(const BitVector &other) noexcept {
    assert(m_size == other.m_size);

    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= static_cast<word_type>(~other.m_words[i]);
    }

    return *this;
  }

  /**
   * \brief Clears bits present in a mask.
   * \code{.cpp}
   * void clear_bits(const BitVector& mask) noexcept;
   * \endcode
   *
   * \param mask Bits to clear.
   *
   * \attention 1. Both vectors must have the same size.
   */
  void clear_bits(const BitVector &mask) noexcept {
    *this -= mask;
  }

  /**
   * \brief Applies binary bitwise operations.
   * \code{.cpp}
   * BitVector operator|(BitVector lhs, const BitVector& rhs);
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   * \param value Operand to complement.
   *
   * \return The resulting bit vector.
   *
   * \attention 1. Binary operations require equal vector sizes.
   */
  [[nodiscard]] friend BitVector operator|(BitVector lhs,
                                           const BitVector &rhs) {
    lhs |= rhs;
    return lhs;
  }

  /**
   * \brief Computes bitwise and.
   * \code{.cpp}
   * [[nodiscard]] friend BitVector operator&(...);
   * \endcode
   * \param other Other bit vector.
   * \attention 1. Both vectors must have the same size.
   */
  [[nodiscard]] friend BitVector operator&(BitVector lhs,
                                           const BitVector &rhs) {
    lhs &= rhs;
    return lhs;
  }

  /**
   * \brief Computes bitwise xor.
   * \code{.cpp}
   * [[nodiscard]] friend BitVector operator^(...);
   * \endcode
   * \param other Other bit vector.
   * \attention 1. Both vectors must have the same size.
   */
  [[nodiscard]] friend BitVector operator^(BitVector lhs,
                                           const BitVector &rhs) {
    lhs ^= rhs;
    return lhs;
  }

  /**
   * \brief Computes masked subtraction.
   * \code{.cpp}
   * [[nodiscard]] friend BitVector operator-(...);
   * \endcode
   * \param other Other bit vector.
   * \attention 1. Both vectors must have the same size.
   */
  [[nodiscard]] friend BitVector operator-(BitVector lhs,
                                           const BitVector &rhs) {
    lhs -= rhs;
    return lhs;
  }

  /**
   * \brief Complements all bits.
   * \code{.cpp}
   * [[nodiscard]] friend BitVector operator~(...);
   * \endcode
   * \param other Other bit vector.
   * \attention 1. Both vectors must have the same size.
   */
  [[nodiscard]] friend BitVector operator~(BitVector value) noexcept {
    value.flip_all();
    return value;
  }

  /**
   * \brief Compares two bit vectors.
   * \code{.cpp}
   * bool operator==(const BitVector& lhs, const BitVector& rhs) noexcept;
   * \endcode
   *
   * \param lhs Left operand.
   * \param rhs Right operand.
   *
   * \return Whether both vectors have equal sizes and bits.
   */
  [[nodiscard]] friend bool operator==(const BitVector &lhs,
                                       const BitVector &rhs) noexcept {
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
  [[no_unique_address]] allocator_type m_allocator;
  size_t m_size = 0;
  word_type *m_words = nullptr;
};

} // namespace strobe

namespace std {

template <typename Word, strobe::Allocator Alloc>
struct hash<strobe::BitVector<Word, Alloc>> {
  size_t operator()(const strobe::BitVector<Word, Alloc> &bits) const
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
struct formatter<strobe::BitVector<Word, Alloc>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const strobe::BitVector<Word, Alloc> &bits,
              FormatContext &ctx) const {
    auto out = ctx.out();
    for (size_t i = bits.size(); i != 0; --i) {
      *out++ = bits.test(i - 1) ? '1' : '0';
    }
    return out;
  }
};

} // namespace fmt
