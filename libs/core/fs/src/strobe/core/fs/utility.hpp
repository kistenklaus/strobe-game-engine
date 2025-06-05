#pragma once

#include <span>
namespace strobe::fs::details {

std::size_t normalize_path_inplace(std::span<char> nonNullTerminatedPath);

}
