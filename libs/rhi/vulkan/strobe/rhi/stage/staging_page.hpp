#pragma once

#include "strobe/rhi/objects/buffer.hpp"
namespace strobe::rhi {

struct StagingPage {
  Buffer buffer{};
  StagingPage *next = nullptr;

  uint64_t size() const noexcept {
    return buffer.size();
  }
};

} // namespace strobe::rhi
