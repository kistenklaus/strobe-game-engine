#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <mutex>
#include <type_traits>

namespace strobe {

template <Allocator A> class BitmapIndexPool {
public:
  using allocator = std::remove_cvref_t<A>;
  using bitmap = std::size_t;

  static constexpr std::size_t INVALID_INDEX =
      std::numeric_limits<std::size_t>::max();

  explicit BitmapIndexPool(std::size_t size, const allocator &alloc) noexcept
      : m_allocator(alloc) {
    resize_unlocked(size);
  }

  ~BitmapIndexPool() noexcept {
    if (m_words != nullptr) {
      allocator_traits::template deallocate<bitmap>(m_allocator, m_words,
                                                    m_wordCount);
    }
  }

  BitmapIndexPool(const BitmapIndexPool &) = delete;
  BitmapIndexPool &operator=(const BitmapIndexPool &) = delete;
  BitmapIndexPool(BitmapIndexPool &&) = delete;
  BitmapIndexPool &operator=(BitmapIndexPool &&) = delete;

  [[nodiscard]] std::size_t alloc(uint32_t count = 1) noexcept {
    std::lock_guard lock{m_mutex};

    const std::size_t requested = count;
    if (requested == 0 || requested > m_freeCount) {
      return INVALID_INDEX;
    }

    const std::size_t index =
        find_free_run_unlocked(m_firstFree, m_size, requested);

    if (index == INVALID_INDEX) {
      return INVALID_INDEX;
    }

    set_range_unlocked(index, requested);
    m_freeCount -= requested;

    if (index == m_firstFree) {
      const std::size_t next =
          find_free_run_unlocked(index + requested, m_size, 1);
      m_firstFree = next == INVALID_INDEX ? m_size : next;
    }

    return index;
  }

  void free(std::size_t index, uint32_t count = 1) noexcept {
    std::lock_guard lock{m_mutex};

    const std::size_t released = count;
    assert(released != 0);
    assert(index <= m_size);
    assert(released <= m_size - index);

    clear_range_unlocked(index, released);
    m_freeCount += released;
    m_firstFree = std::min(m_firstFree, index);
  }

  // Heap growth never needs to move an allocated index, so shrinking is not
  // supported. Newly appended indices are free.
  void resize(std::size_t newSize) noexcept {
    std::lock_guard lock{m_mutex};

    assert(newSize >= m_size);
    if (newSize <= m_size) {
      return;
    }

    resize_unlocked(newSize);
  }

private:
  using allocator_traits = AllocatorTraits<allocator>;

  static constexpr std::size_t BITS_PER_WORD =
      std::numeric_limits<bitmap>::digits;

  [[nodiscard]] static constexpr std::size_t
  word_count(std::size_t bitCount) noexcept {
    return bitCount / BITS_PER_WORD +
           static_cast<std::size_t>(bitCount % BITS_PER_WORD != 0);
  }

  [[nodiscard]] static constexpr bitmap
  low_mask(std::size_t bitCount) noexcept {
    assert(bitCount != 0 && bitCount <= BITS_PER_WORD);
    if (bitCount == BITS_PER_WORD) {
      return ~bitmap{0};
    }
    return (bitmap{1} << bitCount) - 1;
  }

  [[nodiscard]] std::size_t
  find_free_run_unlocked(std::size_t begin, std::size_t end,
                         std::size_t count) const noexcept {
    assert(begin <= end);
    assert(end <= m_size);
    assert(count != 0);

    std::size_t cursor = begin;
    std::size_t runBegin = begin;
    std::size_t runLength = 0;

    while (cursor < end) {
      const std::size_t wordIndex = cursor / BITS_PER_WORD;
      const std::size_t bitIndex = cursor % BITS_PER_WORD;
      const std::size_t available =
          std::min(BITS_PER_WORD - bitIndex, end - cursor);

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

      const std::size_t leadingFree =
          static_cast<std::size_t>(std::countr_zero(occupied));
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

      // cursor now points at an occupied bit. Skip the complete occupied run
      // in this word and restart the free-run count after it.
      const std::size_t occupiedAvailable =
          std::min(BITS_PER_WORD - cursor % BITS_PER_WORD, end - cursor);
      const bitmap shifted =
          m_words[cursor / BITS_PER_WORD] >> (cursor % BITS_PER_WORD);
      const std::size_t occupiedRun =
          std::min(static_cast<std::size_t>(std::countr_one(shifted)),
                   occupiedAvailable);

      assert(occupiedRun != 0);
      cursor += occupiedRun;
      runLength = 0;
    }

    return INVALID_INDEX;
  }

  void set_range_unlocked(std::size_t index, std::size_t count) noexcept {
    while (count != 0) {
      const std::size_t wordIndex = index / BITS_PER_WORD;
      const std::size_t bitIndex = index % BITS_PER_WORD;
      const std::size_t chunk = std::min(count, BITS_PER_WORD - bitIndex);
      const bitmap mask = low_mask(chunk) << bitIndex;

      assert((m_words[wordIndex] & mask) == 0);
      m_words[wordIndex] |= mask;

      index += chunk;
      count -= chunk;
    }
  }

  void clear_range_unlocked(std::size_t index, std::size_t count) noexcept {
    while (count != 0) {
      const std::size_t wordIndex = index / BITS_PER_WORD;
      const std::size_t bitIndex = index % BITS_PER_WORD;
      const std::size_t chunk = std::min(count, BITS_PER_WORD - bitIndex);
      const bitmap mask = low_mask(chunk) << bitIndex;

      // This catches double frees and mismatched array lengths in debug
      // builds.
      assert((m_words[wordIndex] & mask) == mask);
      m_words[wordIndex] &= ~mask;

      index += chunk;
      count -= chunk;
    }
  }

  void resize_unlocked(std::size_t newSize) noexcept {
    const std::size_t newWordCount = word_count(newSize);

    if (newWordCount != m_wordCount) {
      bitmap *newWords = nullptr;
      if (newWordCount != 0) {
        newWords = allocator_traits::template allocate<bitmap>(m_allocator,
                                                               newWordCount);
        assert(newWords != nullptr);
        std::memset(newWords, 0, newWordCount * sizeof(bitmap));

        if (m_wordCount != 0) {
          std::memcpy(newWords, m_words, m_wordCount * sizeof(bitmap));
        }
      }

      if (m_words != nullptr) {
        allocator_traits::template deallocate<bitmap>(m_allocator, m_words,
                                                      m_wordCount);
      }

      m_words = newWords;
      m_wordCount = newWordCount;
    }

    m_freeCount += newSize - m_size;
    m_size = newSize;
  }

  [[no_unique_address]] allocator m_allocator;
  bitmap *m_words = nullptr;
  std::size_t m_size = 0;
  std::size_t m_wordCount = 0;
  std::size_t m_freeCount = 0;
  // No index below m_firstFree is free. This keeps the usual single-slot
  // allocation path O(1) while retaining deterministic first-fit behavior.
  std::size_t m_firstFree = 0;
  std::mutex m_mutex;
};

} // namespace strobe
