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

template <std::size_t BitCount, typename Word = uint64_t>
  requires(std::unsigned_integral<Word> && !std::same_as<Word, bool>)
class StaticBitset {
public:
  using word_type = Word;

  static_assert(BitCount > 0);

  static constexpr size_t bit_count = BitCount;
  static constexpr size_t word_bits =
      std::numeric_limits<word_type>::digits;
  static constexpr size_t word_count =
      (bit_count + word_bits - 1) / word_bits;

private:
  static constexpr word_type full_word =
      std::numeric_limits<word_type>::max();

  static constexpr size_t tail_bits = bit_count % word_bits;

  static constexpr word_type tail_mask = [] {
    if constexpr (tail_bits == 0) {
      return full_word;
    } else {
      return static_cast<word_type>((word_type{1} << tail_bits) - word_type{1});
    }
  }();

public:
  constexpr StaticBitset() noexcept = default;

  [[nodiscard]] static constexpr StaticBitset full() noexcept {
    StaticBitset result;
    result.set_all();
    return result;
  }

  constexpr void set(size_t index) noexcept {
    assert(index < bit_count);
    m_words[index / word_bits] |=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  constexpr void reset(size_t index) noexcept {
    assert(index < bit_count);
    m_words[index / word_bits] &=
        static_cast<word_type>(~(word_type{1} << (index % word_bits)));
  }

  constexpr void flip(size_t index) noexcept {
    assert(index < bit_count);
    m_words[index / word_bits] ^=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  [[nodiscard]] constexpr bool test(size_t index) const noexcept {
    assert(index < bit_count);
    return (m_words[index / word_bits] &
            static_cast<word_type>(word_type{1} << (index % word_bits))) != 0;
  }

  [[nodiscard]] constexpr bool operator[](size_t index) const noexcept {
    return test(index);
  }

  constexpr void set_all() noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] = full_word;
    }

    if constexpr (tail_bits != 0) {
      m_words[word_count - 1] = tail_mask;
    }
  }

  constexpr void reset_all() noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] = 0;
    }
  }

  constexpr void flip_all() noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] = static_cast<word_type>(~m_words[i]);
    }

    if constexpr (tail_bits != 0) {
      m_words[word_count - 1] &= tail_mask;
    }
  }

  [[nodiscard]] constexpr bool any() const noexcept {
    word_type merged = 0;

    for (size_t i = 0; i < word_count; ++i) {
      merged |= m_words[i];
    }

    return merged != 0;
  }

  [[nodiscard]] constexpr bool none() const noexcept { return !any(); }

  [[nodiscard]] constexpr bool all() const noexcept {
    word_type missing = 0;

    for (size_t i = 0; i + 1 < word_count; ++i) {
      missing |= static_cast<word_type>(~m_words[i]);
    }

    missing |= static_cast<word_type>(m_words[word_count - 1] ^ tail_mask);

    return missing == 0;
  }

  [[nodiscard]] constexpr size_t count() const noexcept {
    size_t result = 0;

    for (size_t i = 0; i < word_count; ++i) {
      result += static_cast<size_t>(std::popcount(m_words[i]));
    }

    return result;
  }

  template <typename Fn>
  constexpr void for_each_set_bit(Fn &&fn) const
      noexcept(noexcept(std::declval<Fn &>()(size_t{}))) {
    for (size_t wordIdx = 0; wordIdx < word_count; ++wordIdx) {
      word_type word = m_words[wordIdx];

      while (word != 0) {
        const size_t bit =
            static_cast<size_t>(std::countr_zero(word));
        fn(wordIdx * word_bits + bit);
        word &= static_cast<word_type>(word - 1);
      }
    }
  }

  [[nodiscard]] constexpr word_type word(size_t index) const noexcept {
    assert(index < word_count);
    return m_words[index];
  }

  constexpr void set_word(size_t index, word_type value) noexcept {
    assert(index < word_count);

    if constexpr (tail_bits != 0) {
      if (index + 1 == word_count) {
        value &= tail_mask;
      }
    }

    m_words[index] = value;
  }

  [[nodiscard]] constexpr const word_type *data() const noexcept {
    return m_words.data();
  }

  constexpr StaticBitset &operator|=(const StaticBitset &other) noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] |= other.m_words[i];
    }

    return *this;
  }

  constexpr StaticBitset &operator&=(const StaticBitset &other) noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] &= other.m_words[i];
    }

    return *this;
  }

  constexpr StaticBitset &operator^=(const StaticBitset &other) noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] ^= other.m_words[i];
    }

    return *this;
  }

  constexpr StaticBitset &operator-=(const StaticBitset &other) noexcept {
    for (size_t i = 0; i < word_count; ++i) {
      m_words[i] &= static_cast<word_type>(~other.m_words[i]);
    }

    return *this;
  }

  constexpr void clear_bits(const StaticBitset &mask) noexcept {
    *this -= mask;
  }

  [[nodiscard]] friend constexpr StaticBitset
  operator|(StaticBitset lhs, const StaticBitset &rhs) noexcept {
    lhs |= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr StaticBitset
  operator&(StaticBitset lhs, const StaticBitset &rhs) noexcept {
    lhs &= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr StaticBitset
  operator^(StaticBitset lhs, const StaticBitset &rhs) noexcept {
    lhs ^= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr StaticBitset
  operator-(StaticBitset lhs, const StaticBitset &rhs) noexcept {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr StaticBitset
  operator~(StaticBitset value) noexcept {
    value.flip_all();
    return value;
  }

  [[nodiscard]] friend constexpr bool
  operator==(const StaticBitset &lhs, const StaticBitset &rhs) noexcept {
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
