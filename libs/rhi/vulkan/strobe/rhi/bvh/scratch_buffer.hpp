#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

class ScratchBuffer : public Object<ScratchBuffer> {
  friend class Object<ScratchBuffer>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit ScratchBuffer(void *handle) noexcept : Object(handle) {}

  Buffer buffer() const noexcept;
  void require(uint64_t size) const noexcept;
};

} // namespace strobe::rhi
