#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"
#include "strobe/core/memory/stl_allocator.hpp"
#include <string>

namespace strobe {

template <Allocator A = strobe::Mallocator, typename C = char>
using String = std::basic_string<C, std::char_traits<C>, StlAllocator<C, A>>;

}
