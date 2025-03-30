#pragma once

#include <chrono>
#include <cstddef>

#include "core/AtomicRefCounter.hpp"
#include "resource/file/Path.hpp"

namespace strobe::resource::detail {

struct FileControlBlock {
  AtomicRefCounter<std::size_t> referenceCounter;
  std::size_t size;
  const std::byte* data;
  strobe::resource::Path path;
  std::atomic<std::chrono::high_resolution_clock::time_point> lastUsed;
};

}  // namespace strobe::resource::detail
