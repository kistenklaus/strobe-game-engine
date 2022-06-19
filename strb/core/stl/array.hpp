#pragma once
#include <array>
#include <cinttypes>

namespace strb {

template <typename T, const uint64_t N> using array = std::array<T, N>;

}
