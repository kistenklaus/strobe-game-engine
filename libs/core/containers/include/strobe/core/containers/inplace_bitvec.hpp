#pragma once

#include <array>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

namespace strobe {

template <size_t Capacity, typename Word = uint64_t>
  requires(std::unsigned_integral<Word> && !std::same_as<Word, bool>)
class InplaceBitVector {
public:
  using word_type = Word;

  static constexpr size_t capacity = Capacity;
  static constexpr size_t word_bits = std::numeric_limits<word_type>::digits;
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
  constexpr InplaceBitVector() noexcept = default;

  explicit constexpr InplaceBitVector(size_t size) noexcept : m_size(size) {
    assert(size <= capacity);
  }

  [[nodiscard]] static constexpr InplaceBitVector full(size_t size) noexcept {
    InplaceBitVector result(size);
    result.set_all();
    return result;
  }

  [[nodiscard]] constexpr size_t size() const noexcept { return m_size; }
  [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }
  [[nodiscard]] constexpr size_t word_count() const noexcept {
    return words_for(m_size);
  }

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

    // Bits beyond m_size are always zero, including when growing within
    // the same word.
    m_size = newSize;
  }

  constexpr void set(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] |=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  constexpr void reset(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] &=
        static_cast<word_type>(~(word_type{1} << (index % word_bits)));
  }

  constexpr void flip(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] ^=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  [[nodiscard]] constexpr bool test(size_t index) const noexcept {
    assert(index < m_size);
    return (m_words[index / word_bits] &
            static_cast<word_type>(word_type{1} << (index % word_bits))) != 0;
  }

  [[nodiscard]] constexpr bool operator[](size_t index) const noexcept {
    return test(index);
  }

  constexpr void set_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = full_word;
    }
    if (m_size % word_bits != 0) {
      m_words[word_count() - 1] &= last_mask();
    }
  }

  constexpr void reset_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = 0;
    }
  }

  constexpr void flip_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = static_cast<word_type>(~m_words[i]);
    }
    if (m_size % word_bits != 0) {
      m_words[word_count() - 1] &= last_mask();
    }
  }

  [[nodiscard]] constexpr bool any() const noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      if (m_words[i] != 0) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] constexpr bool none() const noexcept { return !any(); }

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

  [[nodiscard]] constexpr size_t count() const noexcept {
    size_t result = 0;
    for (size_t i = 0; i < word_count(); ++i) {
      result += static_cast<size_t>(std::popcount(m_words[i]));
    }
    return result;
  }

  template <typename Fn>
  constexpr void for_each_set_bit(Fn &&fn) const
      noexcept(noexcept(std::declval<Fn &>()(size_t{}))) {
    for (size_t wordIdx = 0; wordIdx < word_count(); ++wordIdx) {
      word_type value = m_words[wordIdx];
      while (value != 0) {
        const size_t bit = static_cast<size_t>(std::countr_zero(value));
        fn(wordIdx * word_bits + bit);
        value &= static_cast<word_type>(value - 1);
      }
    }
  }

  [[nodiscard]] constexpr word_type word(size_t index) const noexcept {
    assert(index < word_count());
    return m_words[index];
  }

  constexpr void set_word(size_t index, word_type value) noexcept {
    assert(index < word_count());
    if (index + 1 == word_count()) {
      value &= last_mask();
    }
    m_words[index] = value;
  }

  [[nodiscard]] constexpr const word_type *data() const noexcept {
    return m_words.data();
  }

  constexpr InplaceBitVector &operator|=(const InplaceBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] |= other.m_words[i];
    }
    return *this;
  }

  constexpr InplaceBitVector &operator&=(const InplaceBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= other.m_words[i];
    }
    return *this;
  }

  constexpr InplaceBitVector &operator^=(const InplaceBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] ^= other.m_words[i];
    }
    return *this;
  }

  constexpr InplaceBitVector &operator-=(const InplaceBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= static_cast<word_type>(~other.m_words[i]);
    }
    return *this;
  }

  constexpr void clear_bits(const InplaceBitVector &mask) noexcept {
    *this -= mask;
  }

  [[nodiscard]] friend constexpr InplaceBitVector
  operator|(InplaceBitVector lhs, const InplaceBitVector &rhs) noexcept {
    lhs |= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr InplaceBitVector
  operator&(InplaceBitVector lhs, const InplaceBitVector &rhs) noexcept {
    lhs &= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr InplaceBitVector
  operator^(InplaceBitVector lhs, const InplaceBitVector &rhs) noexcept {
    lhs ^= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr InplaceBitVector
  operator-(InplaceBitVector lhs, const InplaceBitVector &rhs) noexcept {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr InplaceBitVector
  operator~(InplaceBitVector value) noexcept {
    value.flip_all();
    return value;
  }

  [[nodiscard]] friend constexpr bool
  operator==(const InplaceBitVector &lhs, const InplaceBitVector &rhs) noexcept {
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
