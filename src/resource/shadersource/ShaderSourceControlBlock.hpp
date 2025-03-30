#pragma once

#include <cstddef>
#include "core/AtomicRefCounter.hpp"
#include "resource/file/Path.hpp"
namespace strobe::resource::detail {

struct ShaderSourceControlBlock {
  AtomicRefCounter<std::size_t> referenceCounter;

  std::size_t size;
  const uint32_t* spirv;

  strobe::resource::Path path;
  std::atomic<std::chrono::high_resolution_clock::time_point> lastUsed;
  
};

}
