#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include <string>

namespace strobe {

template <Allocator A, typename C = char> using String = std::basic_string<C>;

}
