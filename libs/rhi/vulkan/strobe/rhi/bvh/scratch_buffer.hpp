#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

class ScratchBuffer : Object<ScratchBuffer> {
public:
  explicit ScratchBuffer(void *handle) noexcept : Object(handle) {}
  ScratchBuffer() noexcept : Object(nullptr) {}
  ScratchBuffer(const ScratchBuffer &) noexcept;
  ScratchBuffer(ScratchBuffer &&) noexcept;
  ScratchBuffer &operator=(const ScratchBuffer &) noexcept;
  ScratchBuffer &operator=(ScratchBuffer &&) noexcept;
  ~ScratchBuffer() noexcept;

  Buffer scratch() const noexcept;
  void require(uint64_t size) const noexcept;
};

} // namespace strobe::rhi
