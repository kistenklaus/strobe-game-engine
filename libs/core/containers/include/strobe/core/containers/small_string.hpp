#pragma once

#include "strobe/core/containers/small_vector.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe {

template <size_t InlineCapacity = 23, Allocator A = Mallocator,
          typename C = char>
class SmallString {
public:
  using value_type = C;
  using traits_type = std::char_traits<C>;
  using allocator_type = A;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reference = C &;
  using const_reference = const C &;
  using pointer = C *;
  using const_pointer = const C *;
  using iterator = C *;
  using const_iterator = const C *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using view_type = std::basic_string_view<C, traits_type>;

  static constexpr size_type npos = size_type(-1);

  explicit SmallString(const A &alloc = {}) : m_chars(alloc) {
    m_chars.push_back(C{});
  }

  SmallString(view_type value, const A &alloc = {}) : SmallString(alloc) {
    append(value);
  }

  SmallString(const C *value, const A &alloc = {})
      : SmallString(view_type(value), alloc) {}

  SmallString(size_type count, C value, const A &alloc = {})
      : SmallString(alloc) {
    append(count, value);
  }

  SmallString(const SmallString &) = default;

  SmallString(SmallString &&other) noexcept(
      std::is_nothrow_move_constructible_v<decltype(m_chars)>)
      : m_chars(std::move(other.m_chars)) {
    other.m_chars.push_back(C{});
  }

  SmallString &operator=(const SmallString &) = default;

  SmallString &operator=(SmallString &&other) noexcept(
      std::is_nothrow_move_assignable_v<decltype(m_chars)>) {
    if (this != &other) {
      m_chars = std::move(other.m_chars);
      other.m_chars.push_back(C{});
    }
    return *this;
  }

  SmallString &operator=(view_type value) { return assign(value); }

  SmallString &operator=(const C *value) { return assign(view_type(value)); }

  [[nodiscard]] size_type size() const noexcept { return m_chars.size() - 1; }
  [[nodiscard]] size_type length() const noexcept { return size(); }
  [[nodiscard]] bool empty() const noexcept { return size() == 0; }

  [[nodiscard]] size_type capacity() const noexcept {
    return m_chars.capacity() - 1;
  }

  [[nodiscard]] static constexpr size_type inline_capacity() noexcept {
    return InlineCapacity;
  }

  void reserve(size_type count) {
    assert(count < std::numeric_limits<size_type>::max());
    m_chars.reserve(count + 1);
  }

  [[nodiscard]] pointer data() noexcept { return m_chars.data(); }
  [[nodiscard]] const_pointer data() const noexcept { return m_chars.data(); }
  [[nodiscard]] const_pointer c_str() const noexcept { return data(); }

  [[nodiscard]] iterator begin() noexcept { return data(); }
  [[nodiscard]] const_iterator begin() const noexcept { return data(); }
  [[nodiscard]] const_iterator cbegin() const noexcept { return data(); }

  [[nodiscard]] iterator end() noexcept { return data() + size(); }
  [[nodiscard]] const_iterator end() const noexcept { return data() + size(); }
  [[nodiscard]] const_iterator cend() const noexcept { return data() + size(); }

  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  [[nodiscard]] reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  [[nodiscard]] reference operator[](size_type index) noexcept {
    assert(index <= size());
    return data()[index];
  }

  [[nodiscard]] const_reference operator[](size_type index) const noexcept {
    assert(index <= size());
    return data()[index];
  }

  [[nodiscard]] reference front() noexcept {
    assert(!empty());
    return data()[0];
  }

  [[nodiscard]] const_reference front() const noexcept {
    assert(!empty());
    return data()[0];
  }

  [[nodiscard]] reference back() noexcept {
    assert(!empty());
    return data()[size() - 1];
  }

  [[nodiscard]] const_reference back() const noexcept {
    assert(!empty());
    return data()[size() - 1];
  }

  [[nodiscard]] view_type view() const noexcept { return {data(), size()}; }
  [[nodiscard]] operator view_type() const noexcept { return view(); }

  void clear() noexcept {
    m_chars.resize(1);
    m_chars[0] = C{};
  }

  void push_back(C value) {
    const size_type old_size = size();
    m_chars.resize(old_size + 2);
    data()[old_size] = value;
    data()[old_size + 1] = C{};
  }

  void pop_back() noexcept {
    assert(!empty());
    m_chars.pop_back();
    m_chars.back() = C{};
  }

  void resize(size_type count, C value = C{}) {
    assert(count < std::numeric_limits<size_type>::max());

    const size_type old_size = size();
    m_chars.resize(count + 1);

    if (count > old_size) {
      traits_type::assign(data() + old_size, count - old_size, value);
    }
    data()[count] = C{};
  }

  SmallString &assign(view_type value) {
    const size_type count = value.size();
    assert(count < std::numeric_limits<size_type>::max());

    if (aliases(value)) {
      // Source may begin inside the current string. Copy before shrinking.
      traits_type::move(data(), value.data(), count);
      m_chars.resize(count + 1);
    } else {
      m_chars.resize(count + 1);
      if (count != 0) {
        traits_type::copy(data(), value.data(), count);
      }
    }

    data()[count] = C{};
    return *this;
  }

  SmallString &append(view_type value) {
    const size_type old_size = size();
    const size_type count = value.size();
    assert(count < std::numeric_limits<size_type>::max() - old_size);

    if (count == 0) {
      return *this;
    }

    const bool overlapping = aliases(value);
    const size_type source_offset =
        overlapping ? static_cast<size_type>(value.data() - data()) : 0;

    m_chars.resize(old_size + count + 1);

    const C *source = overlapping ? data() + source_offset : value.data();
    traits_type::move(data() + old_size, source, count);
    data()[old_size + count] = C{};
    return *this;
  }

  SmallString &append(const C *value) { return append(view_type(value)); }

  SmallString &append(size_type count, C value) {
    const size_type old_size = size();
    assert(count < std::numeric_limits<size_type>::max() - old_size);

    if (count == 0) {
      return *this;
    }

    m_chars.resize(old_size + count + 1);
    traits_type::assign(data() + old_size, count, value);
    data()[old_size + count] = C{};
    return *this;
  }

  SmallString &operator+=(view_type value) { return append(value); }
  SmallString &operator+=(const C *value) { return append(value); }

  SmallString &operator+=(C value) {
    push_back(value);
    return *this;
  }

  SmallString &insert(size_type pos, view_type value) {
    assert(pos <= size());

    if (aliases(value)) {
      SmallString snapshot(value);
      return insert(pos, snapshot.view());
    }

    const size_type old_size = size();
    const size_type count = value.size();
    assert(count < std::numeric_limits<size_type>::max() - old_size);

    if (count == 0) {
      return *this;
    }

    m_chars.resize(old_size + count + 1);
    traits_type::move(data() + pos + count, data() + pos, old_size - pos + 1);
    traits_type::copy(data() + pos, value.data(), count);
    return *this;
  }

  SmallString &insert(size_type pos, const C *value) {
    return insert(pos, view_type(value));
  }

  SmallString &erase(size_type pos = 0, size_type count = npos) {
    assert(pos <= size());

    const size_type old_size = size();
    const size_type removed = std::min(count, old_size - pos);

    if (removed != 0) {
      traits_type::move(data() + pos, data() + pos + removed,
                        old_size - pos - removed + 1);
      m_chars.resize(old_size - removed + 1);
    }
    return *this;
  }

  SmallString &replace(size_type pos, size_type count, view_type value) {
    assert(pos <= size());

    if (aliases(value)) {
      SmallString snapshot(value);
      return replace(pos, count, snapshot.view());
    }

    const size_type old_size = size();
    const size_type removed = std::min(count, old_size - pos);
    const size_type added = value.size();
    assert(added <
           std::numeric_limits<size_type>::max() - (old_size - removed));

    const size_type new_size = old_size - removed + added;

    if (new_size > old_size) {
      m_chars.resize(new_size + 1);
    }

    if (added != removed) {
      traits_type::move(data() + pos + added, data() + pos + removed,
                        old_size - pos - removed + 1);
    }
    if (added != 0) {
      traits_type::copy(data() + pos, value.data(), added);
    }

    if (new_size < old_size) {
      m_chars.resize(new_size + 1);
    }
    return *this;
  }

  [[nodiscard]] SmallString substr(size_type pos = 0,
                                   size_type count = npos) const {
    assert(pos <= size());
    return SmallString(view().substr(pos, count));
  }

  [[nodiscard]] int compare(view_type other) const noexcept {
    return view().compare(other);
  }

  [[nodiscard]] size_type find(view_type value,
                               size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] size_type find(C value, size_type pos = 0) const noexcept {
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

  [[nodiscard]] friend bool operator==(const SmallString &lhs,
                                       const SmallString &rhs) noexcept {
    return lhs.view() == rhs.view();
  }

  [[nodiscard]] friend bool operator==(const SmallString &lhs,
                                       view_type rhs) noexcept {
    return lhs.view() == rhs;
  }

  [[nodiscard]] friend auto operator<=>(const SmallString &lhs,
                                        const SmallString &rhs) noexcept {
    return lhs.view() <=> rhs.view();
  }

private:
  [[nodiscard]] bool aliases(view_type value) const noexcept {
    if (value.empty()) {
      return false;
    }

    // std::less provides an ordering for pointers into different objects.
    std::less<const C *> less;
    return !less(value.data(), data()) &&
           less(value.data(), data() + m_chars.size());
  }

  SmallVector<C, InlineCapacity + 1, A> m_chars;
};

} // namespace strobe
