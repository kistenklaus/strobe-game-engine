#pragma once

#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <iterator>
#include <string_view>

namespace strobe {

template <size_t N, typename C = char> class Str {
public:
  using value_type = C;
  using traits_type = std::char_traits<C>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_reference = const C &;
  using const_pointer = const C *;
  using const_iterator = const C *;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using view_type = std::basic_string_view<C, traits_type>;

  static constexpr size_type npos = size_type(-1);

  constexpr Str() noexcept = default;

  constexpr Str(const C (&literal)[N + 1]) noexcept {
    traits_type::copy(m_data.data(), literal, N);
    m_data[N] = C{};
  }

  explicit constexpr Str(view_type value) noexcept {
    assert(value.size() == N);
    if constexpr (N != 0) {
      traits_type::copy(m_data.data(), value.data(), N);
    }
    m_data[N] = C{};
  }

  [[nodiscard]] static constexpr size_type size() noexcept { return N; }
  [[nodiscard]] static constexpr size_type length() noexcept { return N; }
  [[nodiscard]] static constexpr bool empty() noexcept { return N == 0; }

  [[nodiscard]] constexpr const_pointer data() const noexcept {
    return m_data.data();
  }

  [[nodiscard]] constexpr const_pointer c_str() const noexcept {
    return data();
  }

  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return data();
  }

  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return data() + N;
  }

  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return begin();
  }

  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return end();
  }

  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  [[nodiscard]] constexpr const_reference
  operator[](size_type index) const noexcept {
    assert(index <= N);
    return m_data[index];
  }

  [[nodiscard]] constexpr const_reference front() const noexcept {
    static_assert(N != 0);
    return m_data[0];
  }

  [[nodiscard]] constexpr const_reference back() const noexcept {
    static_assert(N != 0);
    return m_data[N - 1];
  }

  [[nodiscard]] constexpr view_type view() const noexcept {
    return {data(), N};
  }

  [[nodiscard]] constexpr operator view_type() const noexcept {
    return view();
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
    return view().find(value) != npos;
  }

  [[nodiscard]] constexpr view_type
  substr(size_type pos = 0, size_type count = npos) const noexcept {
    assert(pos <= N);
    return view().substr(pos, count);
  }

private:
  std::array<C, N + 1> m_data{};
};

template <typename C, size_t M>
Str(const C (&)[M]) -> Str<M - 1, C>;

template <size_t N, size_t M, typename C>
[[nodiscard]] constexpr bool operator==(const Str<N, C> &lhs,
                                        const Str<M, C> &rhs) noexcept {
  return lhs.view() == rhs.view();
}

template <size_t N, size_t M, typename C>
[[nodiscard]] constexpr auto operator<=>(const Str<N, C> &lhs,
                                         const Str<M, C> &rhs) noexcept {
  return lhs.view() <=> rhs.view();
}

} // namespace strobe
