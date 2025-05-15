#pragma once

#include "strobe/core/containers/vector.hpp"
namespace strobe {

// TODO actually implement SVO.
template <typename T, Allocator A, std::size_t MinSVOCapacity = 8>
using SmallVector = Vector<T, A>;

}  // namespace strobe
