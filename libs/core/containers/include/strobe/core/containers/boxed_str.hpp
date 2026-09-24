#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe {

template <Allocator A = Mallocator, typename C = char>
class BoxedStr {
public:
  using value_type = C;
  using traits_type = std::char_traits<C>;
  using allocator_type = std::remove_cvref_t<A>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_reference = const C &;
  using const_pointer = const C *;
  using const_iterator = const C *;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using view_type = std::basic_string_view<C, traits_type>;

  static constexpr size_type npos = size_type(-1);

private:
  struct Header {
    size_type size;
    [[no_unique_address]] allocator_type allocator;
  };

  struct alignas(Header) alignas(C) Unit {
    std::byte byte;
  };

  using allocator_traits = AllocatorTraits<allocator_type>;

  static constexpr size_type data_offset =
      sizeof(Header) / alignof(C) * alignof(C) +
      (sizeof(Header) % alignof(C) != 0 ? alignof(C) : 0);

  [[nodiscard]] static constexpr size_type units_for(size_type size) noexcept {
    assert(size < (std::numeric_limits<size_type>::max() - data_offset) /
                      sizeof(C));

    const size_type bytes = data_offset + (size + 1) * sizeof(C);
    return bytes / sizeof(Unit) + (bytes % sizeof(Unit) != 0);
  }

  [[nodiscard]] static Header *allocate(size_type size,
                                        const allocator_type &alloc) {
    allocator_type local = alloc;
    Unit *block =
        allocator_traits::template allocate<Unit>(local, units_for(size));
    assert(block != nullptr);

    Header *header =
        std::construct_at(reinterpret_cast<Header *>(block), size, local);

    C *chars = elements(header);
    for (size_type i = 0; i <= size; ++i) {
      std::construct_at(chars + i, C{});
    }
    return header;
  }

  [[nodiscard]] static C *elements(Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }

    auto *bytes = reinterpret_cast<std::byte *>(header);
    return reinterpret_cast<C *>(bytes + data_offset);
  }

  [[nodiscard]] static const C *elements(const Header *header) noexcept {
    if (header == nullptr) {
      return nullptr;
    }

    auto *bytes = reinterpret_cast<const std::byte *>(header);
    return reinterpret_cast<const C *>(bytes + data_offset);
  }

  void destroy() noexcept {
    if (m_header == nullptr) {
      return;
    }

    Header *header = m_header;
    const size_type count = header->size;
    allocator_type alloc = header->allocator;

    for (size_type i = 0; i <= count; ++i) {
      std::destroy_at(elements(header) + i);
    }

    std::destroy_at(header);
    allocator_traits::template deallocate<Unit>(
        alloc, reinterpret_cast<Unit *>(header), units_for(count));
    m_header = nullptr;
  }

public:
  explicit BoxedStr(size_type size, const allocator_type &alloc = {})
      : m_header(allocate(size, alloc)) {}

  explicit BoxedStr(view_type value, const allocator_type &alloc = {})
      : m_header(allocate(value.size(), alloc)) {
    if (!value.empty()) {
      traits_type::copy(elements(m_header), value.data(), value.size());
    }
  }

  BoxedStr(const C *value, const allocator_type &alloc = {})
      : BoxedStr(view_type(value), alloc) {}

  BoxedStr(const BoxedStr &other) {
    if (other.m_header == nullptr) {
      return;
    }

    m_header = allocate(other.size(), other.get_allocator());
    if (!other.empty()) {
      traits_type::copy(elements(m_header), other.data(), other.size());
    }
  }

  BoxedStr(BoxedStr &&other) noexcept
      : m_header(std::exchange(other.m_header, nullptr)) {}

  BoxedStr &operator=(const BoxedStr &other) {
    if (this == &other) {
      return *this;
    }

    destroy();
    if (other.m_header == nullptr) {
      return *this;
    }

    m_header = allocate(other.size(), other.get_allocator());
    if (!other.empty()) {
      traits_type::copy(elements(m_header), other.data(), other.size());
    }
    return *this;
  }

  BoxedStr &operator=(BoxedStr &&other) noexcept {
    if (this != &other) {
      destroy();
      m_header = std::exchange(other.m_header, nullptr);
    }
    return *this;
  }

  ~BoxedStr() noexcept { destroy(); }

  [[nodiscard]] size_type size() const noexcept {
    return m_header == nullptr ? 0 : m_header->size;
  }

  [[nodiscard]] size_type length() const noexcept { return size(); }
  [[nodiscard]] bool empty() const noexcept { return size() == 0; }

  [[nodiscard]] const_pointer data() const noexcept {
    return elements(m_header);
  }

  [[nodiscard]] const_pointer c_str() const noexcept { return data(); }

  [[nodiscard]] const_iterator begin() const noexcept { return data(); }

  [[nodiscard]] const_iterator end() const noexcept {
    return empty() ? data() : data() + size();
  }

  [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }
  [[nodiscard]] const_iterator cend() const noexcept { return end(); }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  [[nodiscard]] const_reference operator[](size_type index) const noexcept {
    assert(index <= size());
    assert(m_header != nullptr);
    return data()[index];
  }

  [[nodiscard]] const_reference front() const noexcept {
    assert(!empty());
    return data()[0];
  }

  [[nodiscard]] const_reference back() const noexcept {
    assert(!empty());
    return data()[size() - 1];
  }

  [[nodiscard]] view_type view() const noexcept {
    return {data(), size()};
  }

  [[nodiscard]] operator view_type() const noexcept { return view(); }

  [[nodiscard]] int compare(view_type other) const noexcept {
    return view().compare(other);
  }

  [[nodiscard]] size_type find(view_type value,
                               size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] size_type find(C value,
                               size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] size_type rfind(view_type value,
                                size_type pos = npos) const noexcept {
    return view().rfind(value, pos);
  }

  [[nodiscard]] bool starts_with(view_type value) const noexcept {
    return view().starts_with(value);
  }

  [[nodiscard]] bool ends_with(view_type value) const noexcept {
    return view().ends_with(value);
  }

  [[nodiscard]] bool contains(view_type value) const noexcept {
    return find(value) != npos;
  }

  [[nodiscard]] view_type substr(size_type pos = 0,
                                 size_type count = npos) const {
    assert(pos <= size());
    return view().substr(pos, count);
  }

  [[nodiscard]] allocator_type get_allocator() const {
    assert(m_header != nullptr);
    return m_header->allocator;
  }

  [[nodiscard]] friend bool operator==(const BoxedStr &lhs,
                                       const BoxedStr &rhs) noexcept {
    return lhs.view() == rhs.view();
  }

  [[nodiscard]] friend auto operator<=>(const BoxedStr &lhs,
                                        const BoxedStr &rhs) noexcept {
    return lhs.view() <=> rhs.view();
  }

private:
  Header *m_header = nullptr;
};

} // namespace strobe
