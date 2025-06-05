#pragma once

#include "Path.hpp"
#include "strobe/core/fs/stat.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
namespace strobe::fs {

bool exists(PathView path);

template <Allocator PA> inline bool exists(const Path<PA> &path) {
  return exists(PathView(path));
}

} // namespace strobe::fs
