#pragma once

#include <span>
namespace strobe {

template <typename T, size_t Extent = std::dynamic_extent> using span = std::span<T, Extent>;

}
