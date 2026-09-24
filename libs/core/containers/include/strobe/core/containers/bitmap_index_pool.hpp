#pragma once

#include "strobe/core/containers/invalid_index.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace strobe {

/**
 * \ingroup container
 * \brief bitset based index pool.
 *
 * A index pool based on a dynamic fixed size bitset, which allows
 * allocating index ranges.
 * Example
 * \code{cpp}
 * int index = pool.alloc(4);
 * \endcode
 * allocates the values index+0, index+1, index+2, index+3
 * \code{cpp}
 * pool.free(index, 4);
 * \endcode
 * to free the index range
 */
template <Allocator A = Mallocator> class BitmapIndexPool {
public:
  using allocator = std::remove_cvref_t<A>;
  using bitmap = size_t;

  explicit BitmapIndexPool(size_t size, const allocator &alloc = {}) noexcept
      : m_allocator(alloc) {
    resize(size);
  }

  ~BitmapIndexPool() noexcept {
    if (m_words != nullptr) {
      allocator_traits::template deallocate<bitmap>(
          m_allocator, m_words, word_count(m_size));
    }
  }

  BitmapIndexPool(const BitmapIndexPool &) = delete;
  BitmapIndexPool &operator=(const BitmapIndexPool &) = delete;
  BitmapIndexPool(BitmapIndexPool &&) = delete;
  BitmapIndexPool &operator=(BitmapIndexPool &&) = delete;

  [[nodiscard]] size_t alloc(uint32_t count = 1) noexcept {
    const size_t requested = count;
    if (requested == 0 || requested > m_freeCount) {
      return INVALID_INDEX;
    }
    const size_t index = find_free(m_firstFree, m_size, requested);
    if (index == INVALID_INDEX) {
      return INVALID_INDEX;
    }
    set_range(index, requested);
    m_freeCount -= requested;
    if (index == m_firstFree) {
      const size_t next = find_free(index + requested, m_size, 1);
      m_firstFree = next == INVALID_INDEX ? m_size : next;
    }
    return index;
  }

  void free(size_t index, uint32_t count = 1) noexcept {
    const size_t released = count;
    assert(released != 0);
    assert(index <= m_size);
    assert(released <= m_size - index);
    {
      size_t c = released;
      size_t i = index;
      while (c != 0) {
        const size_t wordIndex = i / BITS_PER_WORD;
        const size_t bitIndex = i % BITS_PER_WORD;
        const size_t chunk = std::min(c, BITS_PER_WORD - bitIndex);
        const bitmap mask = low_mask(chunk) << bitIndex;
        assert((m_words[wordIndex] & mask) == mask);
        m_words[wordIndex] &= ~mask;
        i += chunk;
        c -= chunk;
      }
    }
    m_freeCount += released;
    m_firstFree = std::min(m_firstFree, index);
  }

  void resize(size_t newSize) noexcept {
    assert(newSize >= m_size);
    if (newSize <= m_size) {
      return;
    }
    const size_t oldWordCount = word_count(m_size);
    const size_t newWordCount = word_count(newSize);
    if (newWordCount != oldWordCount) {
      bitmap *newWords =
          allocator_traits::template allocate<bitmap>(m_allocator,
                                                      newWordCount);
      assert(newWords != nullptr);
      std::memset(newWords, 0, newWordCount * sizeof(bitmap));
      if (oldWordCount != 0) {
        std::memcpy(newWords, m_words, oldWordCount * sizeof(bitmap));
        allocator_traits::template deallocate<bitmap>(
            m_allocator, m_words, oldWordCount);
      }
      m_words = newWords;
    }
    m_freeCount += newSize - m_size;
    m_size = newSize;
  }

private:
  using allocator_traits = AllocatorTraits<allocator>;

  static constexpr size_t BITS_PER_WORD = std::numeric_limits<bitmap>::digits;

  [[nodiscard]] static constexpr size_t word_count(size_t bitCount) noexcept {
    return bitCount / BITS_PER_WORD +
           static_cast<size_t>(bitCount % BITS_PER_WORD != 0);
  }

  [[nodiscard]] static constexpr bitmap low_mask(size_t bitCount) noexcept {
    assert(bitCount != 0 && bitCount <= BITS_PER_WORD);
    if (bitCount == BITS_PER_WORD) {
      return ~bitmap{0};
    }
    return (bitmap{1} << bitCount) - 1;
  }

  [[nodiscard]] size_t find_free(size_t begin, size_t end,
                                 size_t count) const noexcept {
    assert(begin <= end);
    assert(end <= m_size);
    assert(count != 0);
    size_t cursor = begin;
    size_t runBegin = begin;
    size_t runLength = 0;
    while (cursor < end) {
      const size_t wordIndex = cursor / BITS_PER_WORD;
      const size_t bitIndex = cursor % BITS_PER_WORD;
      const size_t available = std::min(BITS_PER_WORD - bitIndex, end - cursor);
      const bitmap occupied =
          (m_words[wordIndex] >> bitIndex) & low_mask(available);
      if (occupied == 0) {
        if (runLength == 0) {
          runBegin = cursor;
        }
        runLength += available;
        if (runLength >= count) {
          return runBegin;
        }
        cursor += available;
        continue;
      }
      const size_t leadingFree =
          static_cast<size_t>(std::countr_zero(occupied));
      if (leadingFree != 0) {
        if (runLength == 0) {
          runBegin = cursor;
        }
        runLength += leadingFree;
        if (runLength >= count) {
          return runBegin;
        }
        cursor += leadingFree;
      }
      const size_t occupiedAvailable =
          std::min(BITS_PER_WORD - cursor % BITS_PER_WORD, end - cursor);
      const bitmap shifted =
          m_words[cursor / BITS_PER_WORD] >> (cursor % BITS_PER_WORD);
      const size_t occupiedRun = std::min(
          static_cast<size_t>(std::countr_one(shifted)), occupiedAvailable);
      assert(occupiedRun != 0);
      cursor += occupiedRun;
      runLength = 0;
    }
    return INVALID_INDEX;
  }

  void set_range(size_t index, size_t count) noexcept {
    while (count != 0) {
      const size_t wordIndex = index / BITS_PER_WORD;
      const size_t bitIndex = index % BITS_PER_WORD;
      const size_t chunk = std::min(count, BITS_PER_WORD - bitIndex);
      const bitmap mask = low_mask(chunk) << bitIndex;
      assert((m_words[wordIndex] & mask) == 0);
      m_words[wordIndex] |= mask;
      index += chunk;
      count -= chunk;
    }
  }

  [[no_unique_address]] allocator m_allocator;
  bitmap *m_words = nullptr;
  size_t m_size = 0;
  size_t m_freeCount = 0;
  size_t m_firstFree = 0;
};

} // namespace strobe
