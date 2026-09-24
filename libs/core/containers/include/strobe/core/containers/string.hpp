#pragma once

#include "strobe/core/containers/vector.hpp"

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <functional>
#include <iterator>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe {

template <Allocator A = Mallocator, typename C = char> class String {
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

  explicit String(const A &alloc = {}) : m_chars(alloc) {
    m_chars.push_back(C{});
  }

  String(view_type value, const A &alloc = {}) : String(alloc) {
    append(value);
  }

  String(const C *value, const A &alloc = {})
      : String(view_type(value), alloc) {}

  String(size_type count, C value, const A &alloc = {}) : String(alloc) {
    append(count, value);
  }

  String(const String &) = default;

  String(String &&other) : m_chars(std::move(other.m_chars)) {
    other.m_chars.push_back(C{});
  }

  String &operator=(const String &) = default;

  String &operator=(String &&other) {
    if (this != &other) {
      m_chars = std::move(other.m_chars);
      other.m_chars.push_back(C{});
    }
    return *this;
  }

  String &operator=(view_type value) { return assign(value); }

  String &operator=(const C *value) { return assign(view_type(value)); }

  [[nodiscard]] size_type size() const noexcept { return m_chars.size() - 1; }
  [[nodiscard]] size_type length() const noexcept { return size(); }
  [[nodiscard]] bool empty() const noexcept { return size() == 0; }

  [[nodiscard]] size_type capacity() const noexcept {
    return m_chars.capacity() - 1;
  }

  void reserve(size_type count) {
    assert(count < Vector<C, A>::max_size());
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
    assert(old_size < Vector<C, A>::max_size() - 1);

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
    assert(count < Vector<C, A>::max_size());

    const size_type old_size = size();
    m_chars.resize(count + 1);

    if (count > old_size) {
      traits_type::assign(data() + old_size, count - old_size, value);
    }
    data()[count] = C{};
  }

  String &assign(view_type value) {
    const size_type count = value.size();
    assert(count < Vector<C, A>::max_size());

    if (aliases(value)) {
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

  String &append(view_type value) {
    const size_type old_size = size();
    const size_type count = value.size();
    assert(count < Vector<C, A>::max_size() - old_size);

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

  String &append(const C *value) { return append(view_type(value)); }

  String &append(size_type count, C value) {
    const size_type old_size = size();
    assert(count < Vector<C, A>::max_size() - old_size);

    if (count != 0) {
      m_chars.resize(old_size + count + 1);
      traits_type::assign(data() + old_size, count, value);
      data()[old_size + count] = C{};
    }
    return *this;
  }

  String &operator+=(view_type value) { return append(value); }
  String &operator+=(const C *value) { return append(value); }
  String &operator+=(C value) {
    push_back(value);
    return *this;
  }

  String &insert(size_type pos, view_type value) {
    assert(pos <= size());

    if (aliases(value)) {
      String snapshot(value);
      return insert_external(pos, snapshot.view());
    }

    return insert_external(pos, value);
  }

  String &insert(size_type pos, const C *value) {
    return insert(pos, view_type(value));
  }

  String &erase(size_type pos = 0, size_type count = npos) {
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

  String &replace(size_type pos, size_type count, view_type value) {
    assert(pos <= size());
    const size_type removed = std::min(count, size() - pos);

    if (aliases(value)) {
      String snapshot(value);
      return replace_external(pos, removed, snapshot.view());
    }

    return replace_external(pos, removed, value);
  }

  [[nodiscard]] String substr(size_type pos = 0, size_type count = npos) const {
    assert(pos <= size());
    return String(view().substr(pos, count));
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

  [[nodiscard]] friend bool operator==(const String &lhs,
                                       const String &rhs) noexcept {
    return lhs.view() == rhs.view();
  }

  [[nodiscard]] friend bool operator==(const String &lhs,
                                       view_type rhs) noexcept {
    return lhs.view() == rhs;
  }

  [[nodiscard]] friend auto operator<=>(const String &lhs,
                                        const String &rhs) noexcept {
    return lhs.view() <=> rhs.view();
  }

private:
  [[nodiscard]] bool aliases(view_type value) const noexcept {
    if (value.empty()) {
      return false;
    }

    std::less<const C *> less;
    return !less(value.data(), data()) &&
           less(value.data(), data() + m_chars.size());
  }

  String &insert_external(size_type pos, view_type value) {
    const size_type old_size = size();
    const size_type count = value.size();
    assert(count < Vector<C, A>::max_size() - old_size);

    if (count == 0) {
      return *this;
    }

    m_chars.resize(old_size + count + 1);
    traits_type::move(data() + pos + count, data() + pos,
                      old_size - pos + 1); // Includes terminator.
    traits_type::copy(data() + pos, value.data(), count);
    return *this;
  }

  String &replace_external(size_type pos, size_type removed, view_type value) {
    const size_type old_size = size();
    const size_type added = value.size();
    assert(added < Vector<C, A>::max_size() - (old_size - removed));

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

  Vector<C, A> m_chars;
};

} // namespace strobe
