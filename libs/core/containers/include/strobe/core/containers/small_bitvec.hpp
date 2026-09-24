#pragma once

#include "strobe/core/containers/small_vector.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace strobe {

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
  explicit SmallBitVector(const A &alloc = {}) : m_words(alloc) {}

  explicit SmallBitVector(size_t size, const A &alloc = {})
      : m_words(alloc) {
    resize(size);
  }

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

  [[nodiscard]] size_t size() const noexcept { return m_size; }
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
  [[nodiscard]] size_t word_count() const noexcept { return m_words.size(); }

  // Storage capacity is measured in whole words.
  [[nodiscard]] size_t capacity() const noexcept {
    return m_words.capacity() * word_bits;
  }

  [[nodiscard]] static constexpr size_t inline_capacity() noexcept {
    return inline_words * word_bits;
  }

  [[nodiscard]] bool using_inline_storage() const noexcept {
    return m_words.using_inline_storage();
  }

  void reserve(size_t bits) {
    m_words.reserve(words_for(bits));
  }

  void shrink_to_fit() {
    m_words.shrink_to_fit();
  }

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

  void set(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] |=
        static_cast<Word>(Word{1} << (index % word_bits));
  }

  void reset(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] &=
        static_cast<Word>(~(Word{1} << (index % word_bits)));
  }

  void flip(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] ^=
        static_cast<Word>(Word{1} << (index % word_bits));
  }

  [[nodiscard]] bool test(size_t index) const noexcept {
    assert(index < m_size);
    return (m_words[index / word_bits] &
            static_cast<Word>(Word{1} << (index % word_bits))) != 0;
  }

  [[nodiscard]] bool operator[](size_t index) const noexcept {
    return test(index);
  }

  void set_all() noexcept {
    for (Word &word : m_words) {
      word = full_word;
    }
    if (m_size % word_bits != 0) {
      m_words.back() &= last_mask();
    }
  }

  void reset_all() noexcept {
    for (Word &word : m_words) {
      word = 0;
    }
  }

  void flip_all() noexcept {
    for (Word &word : m_words) {
      word = static_cast<Word>(~word);
    }
    if (m_size % word_bits != 0) {
      m_words.back() &= last_mask();
    }
  }

  [[nodiscard]] bool any() const noexcept {
    for (Word word : m_words) {
      if (word != 0) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] bool none() const noexcept { return !any(); }

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

  [[nodiscard]] size_t count() const noexcept {
    size_t result = 0;
    for (Word word : m_words) {
      result += static_cast<size_t>(std::popcount(word));
    }
    return result;
  }

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

  [[nodiscard]] Word word(size_t index) const noexcept {
    assert(index < word_count());
    return m_words[index];
  }

  void set_word(size_t index, Word value) noexcept {
    assert(index < word_count());
    if (index + 1 == word_count()) {
      value &= last_mask();
    }
    m_words[index] = value;
  }

  [[nodiscard]] const Word *data() const noexcept {
    return m_words.data();
  }

  SmallBitVector &operator|=(const SmallBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] |= other.m_words[i];
    }
    return *this;
  }

  SmallBitVector &operator&=(const SmallBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= other.m_words[i];
    }
    return *this;
  }

  SmallBitVector &operator^=(const SmallBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] ^= other.m_words[i];
    }
    return *this;
  }

  SmallBitVector &operator-=(const SmallBitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= static_cast<Word>(~other.m_words[i]);
    }
    return *this;
  }

  void clear_bits(const SmallBitVector &mask) noexcept {
    *this -= mask;
  }

  [[nodiscard]] friend SmallBitVector
  operator|(SmallBitVector lhs, const SmallBitVector &rhs) {
    lhs |= rhs;
    return lhs;
  }

  [[nodiscard]] friend SmallBitVector
  operator&(SmallBitVector lhs, const SmallBitVector &rhs) {
    lhs &= rhs;
    return lhs;
  }

  [[nodiscard]] friend SmallBitVector
  operator^(SmallBitVector lhs, const SmallBitVector &rhs) {
    lhs ^= rhs;
    return lhs;
  }

  [[nodiscard]] friend SmallBitVector
  operator-(SmallBitVector lhs, const SmallBitVector &rhs) {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend SmallBitVector operator~(SmallBitVector value) {
    value.flip_all();
    return value;
  }

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
