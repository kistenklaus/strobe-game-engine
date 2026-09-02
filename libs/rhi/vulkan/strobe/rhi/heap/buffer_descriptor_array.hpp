#pragma once

#include "strobe/rhi/objects/object.hpp"
namespace strobe::rhi {

class BufferDescriptorArray : Object<BufferDescriptorArray> {
public:
  explicit BufferDescriptorArray(void *handle) noexcept : Object(handle) {}
  BufferDescriptorArray() noexcept : Object(nullptr) {}
  BufferDescriptorArray(const BufferDescriptorArray &) noexcept;
  BufferDescriptorArray(BufferDescriptorArray &&) noexcept;
  BufferDescriptorArray &operator=(const BufferDescriptorArray &) noexcept;
  BufferDescriptorArray &operator=(BufferDescriptorArray &&) noexcept;
  ~BufferDescriptorArray() noexcept;
};

} // namespace strobe::rhi
