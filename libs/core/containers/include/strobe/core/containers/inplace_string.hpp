#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <functional>
#include <iterator>
#include <string_view>
#include <type_traits>

namespace strobe {

template <size_t Capacity, typename C = char> class InplaceString {
public:
  using value_type = C;
  using traits_type = std::char_traits<C>;
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
  static_assert(Capacity < npos);

  constexpr InplaceString() noexcept = default;

  constexpr InplaceString(view_type value) { assign(value); }

  constexpr InplaceString(const C *value)
      : InplaceString(view_type(value)) {}

  constexpr InplaceString(size_type count, C value) {
    append(count, value);
  }

  [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }
  [[nodiscard]] constexpr size_type length() const noexcept { return m_size; }
  [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

  [[nodiscard]] static constexpr size_type capacity() noexcept {
    return Capacity;
  }

  [[nodiscard]] constexpr pointer data() noexcept { return m_data.data(); }
  [[nodiscard]] constexpr const_pointer data() const noexcept {
    return m_data.data();
  }
  [[nodiscard]] constexpr const_pointer c_str() const noexcept {
    return data();
  }

  [[nodiscard]] constexpr iterator begin() noexcept { return data(); }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return data();
  }
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return data();
  }

  [[nodiscard]] constexpr iterator end() noexcept { return data() + m_size; }
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return data() + m_size;
  }
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return data() + m_size;
  }

  [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }
  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  [[nodiscard]] constexpr reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }
  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
    assert(index <= m_size);
    return m_data[index];
  }
  [[nodiscard]] constexpr const_reference
  operator[](size_type index) const noexcept {
    assert(index <= m_size);
    return m_data[index];
  }

  [[nodiscard]] constexpr reference front() noexcept {
    assert(!empty());
    return m_data[0];
  }
  [[nodiscard]] constexpr const_reference front() const noexcept {
    assert(!empty());
    return m_data[0];
  }

  [[nodiscard]] constexpr reference back() noexcept {
    assert(!empty());
    return m_data[m_size - 1];
  }
  [[nodiscard]] constexpr const_reference back() const noexcept {
    assert(!empty());
    return m_data[m_size - 1];
  }

  [[nodiscard]] constexpr view_type view() const noexcept {
    return {data(), m_size};
  }

  [[nodiscard]] constexpr operator view_type() const noexcept {
    return view();
  }

  constexpr void clear() noexcept {
    m_size = 0;
    m_data[0] = C{};
  }

  constexpr void push_back(C value) {
    assert(m_size < Capacity && "InplaceString is full");
    m_data[m_size++] = value;
    m_data[m_size] = C{};
  }

  constexpr void pop_back() noexcept {
    assert(!empty());
    m_data[--m_size] = C{};
  }

  constexpr void resize(size_type count, C value = C{}) {
    assert(count <= Capacity && "InplaceString capacity exceeded");

    if (count > m_size) {
      traits_type::assign(data() + m_size, count - m_size, value);
    }

    m_size = count;
    m_data[m_size] = C{};
  }

  constexpr InplaceString &assign(view_type value) {
    assert(value.size() <= Capacity && "InplaceString capacity exceeded");

    if (!value.empty()) {
      traits_type::move(data(), value.data(), value.size());
    }
    m_size = value.size();
    m_data[m_size] = C{};
    return *this;
  }

  constexpr InplaceString &append(view_type value) {
    assert(value.size() <= Capacity - m_size &&
           "InplaceString capacity exceeded");

    if (!value.empty()) {
      // move permits a view into this string.
      traits_type::move(data() + m_size, value.data(), value.size());
      m_size += value.size();
      m_data[m_size] = C{};
    }
    return *this;
  }

  constexpr InplaceString &append(const C *value) {
    return append(view_type(value));
  }

  constexpr InplaceString &append(size_type count, C value) {
    assert(count <= Capacity - m_size &&
           "InplaceString capacity exceeded");

    if (count != 0) {
      traits_type::assign(data() + m_size, count, value);
      m_size += count;
      m_data[m_size] = C{};
    }
    return *this;
  }

  constexpr InplaceString &operator+=(view_type value) {
    return append(value);
  }
  constexpr InplaceString &operator+=(const C *value) {
    return append(value);
  }
  constexpr InplaceString &operator+=(C value) {
    push_back(value);
    return *this;
  }

  constexpr InplaceString &insert(size_type pos, view_type value) {
    assert(pos <= m_size);
    assert(value.size() <= Capacity - m_size &&
           "InplaceString capacity exceeded");

    if (value.empty()) {
      return *this;
    }

    if (aliases(value)) {
      std::array<C, Capacity + 1> snapshot{};
      traits_type::copy(snapshot.data(), value.data(), value.size());
      return insert_external(pos, {snapshot.data(), value.size()});
    }

    return insert_external(pos, value);
  }

  constexpr InplaceString &insert(size_type pos, const C *value) {
    return insert(pos, view_type(value));
  }

  constexpr InplaceString &erase(size_type pos = 0,
                                 size_type count = npos) {
    assert(pos <= m_size);
    const size_type removed = std::min(count, m_size - pos);

    if (removed != 0) {
      traits_type::move(data() + pos, data() + pos + removed,
                        m_size - pos - removed + 1);
      m_size -= removed;
    }
    return *this;
  }

  constexpr InplaceString &replace(size_type pos, size_type count,
                                   view_type value) {
    assert(pos <= m_size);
    const size_type removed = std::min(count, m_size - pos);
    assert(value.size() <= Capacity - (m_size - removed) &&
           "InplaceString capacity exceeded");

    if (aliases(value)) {
      std::array<C, Capacity + 1> snapshot{};
      traits_type::copy(snapshot.data(), value.data(), value.size());
      return replace_external(pos, removed,
                              {snapshot.data(), value.size()});
    }

    return replace_external(pos, removed, value);
  }

  [[nodiscard]] constexpr InplaceString
  substr(size_type pos = 0, size_type count = npos) const {
    assert(pos <= m_size);
    return InplaceString(view().substr(pos, count));
  }

  [[nodiscard]] constexpr int compare(view_type other) const noexcept {
    return view().compare(other);
  }

  [[nodiscard]] constexpr size_type
  find(view_type value, size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] constexpr size_type
  find(C value, size_type pos = 0) const noexcept {
    return view().find(value, pos);
  }

  [[nodiscard]] constexpr size_type
  rfind(view_type value, size_type pos = npos) const noexcept {
    return view().rfind(value, pos);
  }

  [[nodiscard]] constexpr bool starts_with(view_type value) const noexcept {
    return view().starts_with(value);
  }

  [[nodiscard]] constexpr bool ends_with(view_type value) const noexcept {
    return view().ends_with(value);
  }

  [[nodiscard]] constexpr bool contains(view_type value) const noexcept {
    return find(value) != npos;
  }

  [[nodiscard]] friend constexpr bool
  operator==(const InplaceString &lhs,
             const InplaceString &rhs) noexcept {
    return lhs.view() == rhs.view();
  }

  [[nodiscard]] friend constexpr auto
  operator<=>(const InplaceString &lhs,
              const InplaceString &rhs) noexcept {
    return lhs.view() <=> rhs.view();
  }

private:
  [[nodiscard]] constexpr bool aliases(view_type value) const noexcept {
    if (value.empty()) {
      return false;
    }

    if (std::is_constant_evaluated()) {
      // Avoid ordering unrelated pointers during constant evaluation.
      for (size_type i = 0; i <= m_size; ++i) {
        if (value.data() == data() + i) {
          return true;
        }
      }
      return false;
    }

    std::less<const C *> less;
    return !less(value.data(), data()) &&
           less(value.data(), data() + m_size + 1);
  }

  constexpr InplaceString &insert_external(size_type pos, view_type value) {
    traits_type::move(data() + pos + value.size(), data() + pos,
                      m_size - pos + 1); // Includes terminator.
    traits_type::copy(data() + pos, value.data(), value.size());
    m_size += value.size();
    return *this;
  }

  constexpr InplaceString &replace_external(size_type pos, size_type removed,
                                             view_type value) {
    const size_type new_size = m_size - removed + value.size();

    if (value.size() != removed) {
      traits_type::move(data() + pos + value.size(),
                        data() + pos + removed,
                        m_size - pos - removed + 1); // Includes terminator.
    }

    if (!value.empty()) {
      traits_type::copy(data() + pos, value.data(), value.size());
    }

    m_size = new_size;
    return *this;
  }

  std::array<C, Capacity + 1> m_data{};
  size_type m_size = 0;
};

} // namespace strobe
