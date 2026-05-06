#pragma once

#include "strobe/core/containers/vector.hpp"
namespace strobe {

// TODO actually implement SVO.
template <typename T, std::size_t MinSVOCapacity = 8, Allocator A = strobe::Mallocator>
using SmallVector = Vector<T, A>;

}  // namespace strobe
