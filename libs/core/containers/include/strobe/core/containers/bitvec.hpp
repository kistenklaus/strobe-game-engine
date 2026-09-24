#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>

namespace strobe {

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
  BitVector() noexcept = default;

  explicit BitVector(size_t size, const allocator_type &alloc = {})
      : m_allocator(alloc), m_size(size), m_words(allocate(words_for(size))) {
    if (m_words != nullptr) {
      std::memset(m_words, 0, word_count() * sizeof(word_type));
    }
  }

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

  [[nodiscard]] size_t size() const noexcept { return m_size; }
  [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
  [[nodiscard]] size_t word_count() const noexcept { return words_for(m_size); }

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

    // Also clears removed bits when shrinking within the same word.
    if (newCount != 0 && newSize % word_bits != 0) {
      m_words[newCount - 1] &= last_mask();
    }
  }

  void set(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] |=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  void reset(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] &=
        static_cast<word_type>(~(word_type{1} << (index % word_bits)));
  }

  void flip(size_t index) noexcept {
    assert(index < m_size);
    m_words[index / word_bits] ^=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  [[nodiscard]] bool test(size_t index) const noexcept {
    assert(index < m_size);
    return (m_words[index / word_bits] &
            static_cast<word_type>(word_type{1} << (index % word_bits))) != 0;
  }

  [[nodiscard]] bool operator[](size_t index) const noexcept {
    return test(index);
  }

  void set_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = full_word;
    }
    if (m_size % word_bits != 0) {
      m_words[word_count() - 1] &= last_mask();
    }
  }

  void reset_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = 0;
    }
  }

  void flip_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] = static_cast<word_type>(~m_words[i]);
    }
    if (m_size % word_bits != 0) {
      m_words[word_count() - 1] &= last_mask();
    }
  }

  [[nodiscard]] bool any() const noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      if (m_words[i] != 0) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] bool none() const noexcept { return !any(); }

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

  [[nodiscard]] size_t count() const noexcept {
    size_t result = 0;
    for (size_t i = 0; i < word_count(); ++i) {
      result += static_cast<size_t>(std::popcount(m_words[i]));
    }
    return result;
  }

  template <typename Fn>
  void for_each_set_bit(Fn &&fn) const
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

  [[nodiscard]] word_type word(size_t index) const noexcept {
    assert(index < word_count());
    return m_words[index];
  }

  void set_word(size_t index, word_type value) noexcept {
    assert(index < word_count());
    if (index + 1 == word_count()) {
      value &= last_mask();
    }
    m_words[index] = value;
  }

  [[nodiscard]] const word_type *data() const noexcept { return m_words; }

  BitVector &operator|=(const BitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] |= other.m_words[i];
    }
    return *this;
  }

  BitVector &operator&=(const BitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= other.m_words[i];
    }
    return *this;
  }

  BitVector &operator^=(const BitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] ^= other.m_words[i];
    }
    return *this;
  }

  BitVector &operator-=(const BitVector &other) noexcept {
    assert(m_size == other.m_size);
    for (size_t i = 0; i < word_count(); ++i) {
      m_words[i] &= static_cast<word_type>(~other.m_words[i]);
    }
    return *this;
  }

  void clear_bits(const BitVector &mask) noexcept { *this -= mask; }

  [[nodiscard]] friend BitVector operator|(BitVector lhs,
                                           const BitVector &rhs) {
    lhs |= rhs;
    return lhs;
  }

  [[nodiscard]] friend BitVector operator&(BitVector lhs,
                                           const BitVector &rhs) {
    lhs &= rhs;
    return lhs;
  }

  [[nodiscard]] friend BitVector operator^(BitVector lhs,
                                           const BitVector &rhs) {
    lhs ^= rhs;
    return lhs;
  }

  [[nodiscard]] friend BitVector operator-(BitVector lhs,
                                           const BitVector &rhs) {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend BitVector operator~(BitVector value) noexcept {
    value.flip_all();
    return value;
  }

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
