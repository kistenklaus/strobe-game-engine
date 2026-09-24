#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe {

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
  explicit BoxedBitset(size_t size, const allocator_type &alloc = {})
      : m_header(allocate(size, alloc)) {
    for (size_t i = 0; i < word_count(); ++i) {
      std::construct_at(words(m_header) + i, word_type{0});
    }
  }

  [[nodiscard]] static BoxedBitset full(size_t size,
                                        const allocator_type &alloc = {}) {
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

  [[nodiscard]] size_t size() const noexcept {
    return m_header == nullptr ? 0 : m_header->size;
  }

  [[nodiscard]] bool empty() const noexcept { return size() == 0; }

  [[nodiscard]] size_t word_count() const noexcept { return words_for(size()); }

  void set(size_t index) noexcept {
    assert(index < size());
    words(m_header)[index / word_bits] |=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  void reset(size_t index) noexcept {
    assert(index < size());
    words(m_header)[index / word_bits] &=
        static_cast<word_type>(~(word_type{1} << (index % word_bits)));
  }

  void flip(size_t index) noexcept {
    assert(index < size());
    words(m_header)[index / word_bits] ^=
        static_cast<word_type>(word_type{1} << (index % word_bits));
  }

  [[nodiscard]] bool test(size_t index) const noexcept {
    assert(index < size());
    return (words(m_header)[index / word_bits] &
            static_cast<word_type>(word_type{1} << (index % word_bits))) != 0;
  }

  [[nodiscard]] bool operator[](size_t index) const noexcept {
    return test(index);
  }

  void set_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] = full_word;
    }
    if (size() % word_bits != 0) {
      words(m_header)[word_count() - 1] &= last_mask();
    }
  }

  void reset_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] = 0;
    }
  }

  void flip_all() noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] = static_cast<word_type>(~words(m_header)[i]);
    }
    if (size() % word_bits != 0) {
      words(m_header)[word_count() - 1] &= last_mask();
    }
  }

  [[nodiscard]] bool any() const noexcept {
    for (size_t i = 0; i < word_count(); ++i) {
      if (words(m_header)[i] != 0) {
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
      if (words(m_header)[i] != full_word) {
        return false;
      }
    }
    return words(m_header)[count - 1] == last_mask();
  }

  [[nodiscard]] size_t count() const noexcept {
    size_t result = 0;
    for (size_t i = 0; i < word_count(); ++i) {
      result += static_cast<size_t>(std::popcount(words(m_header)[i]));
    }
    return result;
  }

  template <typename Fn>
  void for_each_set_bit(Fn &&fn) const
      noexcept(noexcept(std::declval<Fn &>()(size_t{}))) {
    for (size_t wordIdx = 0; wordIdx < word_count(); ++wordIdx) {
      word_type value = words(m_header)[wordIdx];
      while (value != 0) {
        const size_t bit = static_cast<size_t>(std::countr_zero(value));
        fn(wordIdx * word_bits + bit);
        value &= static_cast<word_type>(value - 1);
      }
    }
  }

  [[nodiscard]] word_type word(size_t index) const noexcept {
    assert(index < word_count());
    return words(m_header)[index];
  }

  void set_word(size_t index, word_type value) noexcept {
    assert(index < word_count());
    if (index + 1 == word_count()) {
      value &= last_mask();
    }
    words(m_header)[index] = value;
  }

  [[nodiscard]] const word_type *data() const noexcept {
    return words(m_header);
  }

  [[nodiscard]] allocator_type get_allocator() const {
    assert(m_header != nullptr);
    return m_header->allocator;
  }

  BoxedBitset &operator|=(const BoxedBitset &other) noexcept {
    assert(size() == other.size());
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] |= other.word(i);
    }
    return *this;
  }

  BoxedBitset &operator&=(const BoxedBitset &other) noexcept {
    assert(size() == other.size());
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] &= other.word(i);
    }
    return *this;
  }

  BoxedBitset &operator^=(const BoxedBitset &other) noexcept {
    assert(size() == other.size());
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] ^= other.word(i);
    }
    return *this;
  }

  BoxedBitset &operator-=(const BoxedBitset &other) noexcept {
    assert(size() == other.size());
    for (size_t i = 0; i < word_count(); ++i) {
      words(m_header)[i] &= static_cast<word_type>(~other.word(i));
    }
    return *this;
  }

  void clear_bits(const BoxedBitset &mask) noexcept { *this -= mask; }

  [[nodiscard]] friend BoxedBitset operator|(BoxedBitset lhs,
                                             const BoxedBitset &rhs) {
    lhs |= rhs;
    return lhs;
  }

  [[nodiscard]] friend BoxedBitset operator&(BoxedBitset lhs,
                                             const BoxedBitset &rhs) {
    lhs &= rhs;
    return lhs;
  }

  [[nodiscard]] friend BoxedBitset operator^(BoxedBitset lhs,
                                             const BoxedBitset &rhs) {
    lhs ^= rhs;
    return lhs;
  }

  [[nodiscard]] friend BoxedBitset operator-(BoxedBitset lhs,
                                             const BoxedBitset &rhs) {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend BoxedBitset operator~(BoxedBitset value) {
    value.flip_all();
    return value;
  }

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
