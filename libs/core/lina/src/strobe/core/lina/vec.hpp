#pragma once

#include <array>
#include <type_traits>
#include <cstddef>
#include <iostream>
#include <tuple> // For std::get

namespace strobe {

namespace details {

// Core vector type definition
template<std::size_t N, typename T>
requires std::is_arithmetic_v<T>
struct vec {
    // Data storage (trivially copyable and movable)
    std::array<T, N> data = {};

    // Default constructor (trivial)
    constexpr vec() noexcept = default;

    // Uniform value constructor (trivial)
    constexpr explicit vec(T value) noexcept {
        data.fill(value);
    }

    // Component-wise constructor for N = 2, 3, 4 (trivial)
    constexpr vec(T x, T y) noexcept requires (N >= 2) : data{x, y} {}
    constexpr vec(T x, T y, T z) noexcept requires (N >= 3) : data{x, y, z} {}
    constexpr vec(T x, T y, T z, T w) noexcept requires (N >= 4) : data{x, y, z, w} {}

    // Array subscript access
    constexpr T& operator[](std::size_t index) noexcept {
        return data[index];
    }

    constexpr const T& operator[](std::size_t index) const noexcept {
        return data[index];
    }

    // Named component access for x, y, z, w (getter functions)
    constexpr T& x() noexcept requires (N >= 1) { return data[0]; }
    constexpr const T& x() const noexcept requires (N >= 1) { return data[0]; }

    constexpr T& y() noexcept requires (N >= 2) { return data[1]; }
    constexpr const T& y() const noexcept requires (N >= 2) { return data[1]; }

    constexpr T& z() noexcept requires (N >= 3) { return data[2]; }
    constexpr const T& z() const noexcept requires (N >= 3) { return data[2]; }

    constexpr T& w() noexcept requires (N >= 4) { return data[3]; }
    constexpr const T& w() const noexcept requires (N >= 4) { return data[3]; }

    // Alternative names for color access
    constexpr T& r() noexcept requires (N >= 1) { return data[0]; }
    constexpr const T& r() const noexcept requires (N >= 1) { return data[0]; }

    constexpr T& g() noexcept requires (N >= 2) { return data[1]; }
    constexpr const T& g() const noexcept requires (N >= 2) { return data[1]; }

    constexpr T& b() noexcept requires (N >= 3) { return data[2]; }
    constexpr const T& b() const noexcept requires (N >= 3) { return data[2]; }

    constexpr T& a() noexcept requires (N >= 4) { return data[3]; }
    constexpr const T& a() const noexcept requires (N >= 4) { return data[3]; }
};

} // namespace details


using vec2 = details::vec<2, float>;
using vec3 = details::vec<3, float>;
using vec4 = details::vec<4, float>;

using dvec2 = details::vec<2, double>;
using dvec3 = details::vec<3, double>;
using dvec4 = details::vec<4, double>;

using uvec2 = details::vec<2, unsigned int>;
using uvec3 = details::vec<3, unsigned int>;
using uvec4 = details::vec<4, unsigned int>;



} // namespace strobe


// Specialize std::tuple_size and std::tuple_element for std::get support
namespace std {
    template<std::size_t N, typename T>
    struct tuple_size<strobe::details::vec<N, T>> : std::integral_constant<std::size_t, N> {};

    template<std::size_t Index, std::size_t N, typename T>
    struct tuple_element<Index, strobe::details::vec<N, T>> {
        using type = T;
    };

    // Overload std::get for vec
    template<std::size_t Index, std::size_t N, typename T>
    constexpr T& get(strobe::details::vec<N, T>& v) noexcept {
        static_assert(Index < N, "Index out of range for vec");
        return v.data[Index];
    }

    template<std::size_t Index, std::size_t N, typename T>
    constexpr const T& get(const strobe::details::vec<N, T>& v) noexcept {
        static_assert(Index < N, "Index out of range for vec");
        return v.data[Index];
    }

    template<std::size_t Index, std::size_t N, typename T>
    constexpr T&& get(strobe::details::vec<N, T>&& v) noexcept {
        static_assert(Index < N, "Index out of range for vec");
        return std::move(v.data[Index]);
    }
}
