#pragma once
#include <bitset>
#include <cinttypes>

namespace strb {

template <const uint64_t N>
using bitset = std::bitset<N>;

}
