#pragma once

#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/vulkan/fence.hpp"
#include <cstdint>
#include <limits>

namespace strobe::rhi {

// Not a object
class Fence : Object<Fence> {
  friend class FencePool;
  explicit Fence(void *handle) noexcept : Object(handle) {}

public:
  Fence() noexcept : Object(nullptr) {}
  Fence(const Fence &) noexcept;
  Fence(Fence &&) noexcept;
  Fence &operator=(const Fence &) noexcept;
  Fence &operator=(Fence &&) noexcept;
  ~Fence() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  bool
  wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const noexcept;

  bool signaled() const noexcept;

  vulkan::Fence fence() const noexcept;
};

} // namespace strobe::rhi
