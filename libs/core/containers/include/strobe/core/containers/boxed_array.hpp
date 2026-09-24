#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <limits>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

namespace strobe {

template <typename T, Allocator Alloc = Mallocator> class BoxedArray {
public:
  using value_type = T;
  using allocator_type = std::remove_cvref_t<Alloc>;
  using size_type = size_t;
  using iterator = T *;
  using const_iterator = const T *;

private:
  struct Header {
    size_type size;
    [[no_unique_address]] allocator_type allocator;
  };

  // AllocatorTraits allocates typed objects. One unit provides the alignment
  // required by both the header and the trailing T elements.
  struct alignas(T) alignas(Header) Unit {
    std::byte byte;
  };

  using allocator_traits = AllocatorTraits<allocator_type>;

  static constexpr size_type data_offset =
      sizeof(Header) / alignof(T) * alignof(T) +
      (sizeof(Header) % alignof(T) != 0 ? alignof(T) : 0);

  [[nodiscard]] static constexpr size_type units_for(size_type size) noexcept {
    assert(size <=
           (std::numeric_limits<size_type>::max() - data_offset) / sizeof(T));
    const size_type bytes = data_offset + size * sizeof(T);
    return bytes / sizeof(Unit) + (bytes % sizeof(Unit) != 0);
  }

  [[nodiscard]] static Header *allocate(size_type size,
                                        const allocator_type &alloc) {
    allocator_type local = alloc;
    Unit *block =
        allocator_traits::template allocate<Unit>(local, units_for(size));
    assert(block != nullptr);
    return std::construct_at(reinterpret_cast<Header *>(block), size, local);
  }

  [[nodiscard]] static T *elements(Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }
    auto *bytes = reinterpret_cast<std::byte *>(header);
    return reinterpret_cast<T *>(bytes + data_offset);
  }

  [[nodiscard]] static const T *elements(const Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }
    auto *bytes = reinterpret_cast<const std::byte *>(header);
    return reinterpret_cast<const T *>(bytes + data_offset);
  }

  void destroy() noexcept {
    if (m_header == nullptr) {
      return;
    }

    Header *header = m_header;
    const size_type n = header->size;
    allocator_type alloc = header->allocator;
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (size_type i = 0; i < n; ++i) {
        std::destroy_at(elements(header) + i);
      }
    }
    std::destroy_at(header);
    allocator_traits::template deallocate<Unit>(
        alloc, reinterpret_cast<Unit *>(header), units_for(n));
    m_header = nullptr;
  }

public:
  explicit BoxedArray(size_type size, const allocator_type &alloc = {})
    requires std::default_initializable<T>
      : m_header(allocate(size, alloc)) {
    for (size_type i = 0; i < size; ++i) {
      std::construct_at(elements(m_header) + i);
    }
  }

  template <std::ranges::forward_range Range>
    requires std::constructible_from<T,
                                     std::ranges::range_reference_t<Range>> &&
             (!std::same_as<std::remove_cvref_t<Range>, BoxedArray>)
  explicit BoxedArray(Range &&range, const allocator_type &alloc = {})
      : m_header(allocate(static_cast<size_type>(std::ranges::distance(range)),
                          alloc)) {
    size_type i = 0;
    for (auto &&value : range) {
      std::construct_at(elements(m_header) + i,
                        std::forward<decltype(value)>(value));
      ++i;
    }
  }

  BoxedArray(const BoxedArray &other)
    requires std::copy_constructible<T>
  {
    if (other.m_header == nullptr) {
      return;
    }
    m_header = allocate(other.size(), other.m_header->allocator);
    for (size_type i = 0; i < size(); ++i) {
      std::construct_at(elements(m_header) + i, other[i]);
    }
  }

  BoxedArray(BoxedArray &&other) noexcept
      : m_header(std::exchange(other.m_header, nullptr)) {}

  BoxedArray &operator=(const BoxedArray &other)
    requires std::copy_constructible<T>
  {
    if (this == &other) {
      return *this;
    }
    destroy();
    if (other.m_header == nullptr) {
      return *this;
    }
    m_header = allocate(other.size(), other.m_header->allocator);
    for (size_type i = 0; i < size(); ++i) {
      std::construct_at(elements(m_header) + i, other[i]);
    }
    return *this;
  }

  BoxedArray &operator=(BoxedArray &&other) noexcept {
    if (this != &other) {
      destroy();
      m_header = std::exchange(other.m_header, nullptr);
    }
    return *this;
  }

  ~BoxedArray() noexcept { destroy(); }

  void resize(size_type newSize)
    requires std::default_initializable<T>
  {
    assert(m_header != nullptr); // A moved-from array has no allocator.
    const size_type oldSize = size();
    if (newSize == oldSize) {
      return;
    }
    Header *newHeader = allocate(newSize, m_header->allocator);
    T *newData = elements(newHeader);
    T *oldData = data();
    const size_type retained = std::min(oldSize, newSize);
    for (size_type i = 0; i < retained; ++i) {
      std::construct_at(newData + i, std::move(oldData[i]));
    }
    for (size_type i = retained; i < newSize; ++i) {
      std::construct_at(newData + i);
    }
    destroy();
    m_header = newHeader;
  }

  [[nodiscard]] size_type size() const noexcept {
    return m_header == nullptr ? 0 : m_header->size;
  }
  [[nodiscard]] bool empty() const noexcept { return size() == 0; }

  [[nodiscard]] T *data() noexcept { return elements(m_header); }
  [[nodiscard]] const T *data() const noexcept { return elements(m_header); }

  [[nodiscard]] iterator begin() noexcept { return data(); }
  [[nodiscard]] const_iterator begin() const noexcept { return data(); }
  [[nodiscard]] iterator end() noexcept {
    return empty() ? data() : data() + size();
  }
  [[nodiscard]] const_iterator end() const noexcept {
    return empty() ? data() : data() + size();
  }

  [[nodiscard]] T &operator[](size_type index) noexcept {
    assert(index < size());
    return data()[index];
  }
  [[nodiscard]] const T &operator[](size_type index) const noexcept {
    assert(index < size());
    return data()[index];
  }

  [[nodiscard]] allocator_type get_allocator() const {
    assert(m_header != nullptr);
    return m_header->allocator;
  }

private:
  Header *m_header = nullptr;
};

} // namespace strobe
