#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/vulkan/fence.hpp"
#include <cstdint>
#include <limits>

namespace strobe::rhi {

// Not a object
class Fence : public Object<Fence> {
  friend class Object<Fence>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit Fence(void *handle) noexcept : Object(handle) {}
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  bool
  wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const noexcept;

  bool signaled() const noexcept;

  vulkan::Fence fence() const noexcept;
};

} // namespace strobe::rhi
