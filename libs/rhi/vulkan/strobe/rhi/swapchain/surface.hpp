#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/vulkan/surface.hpp"
namespace strobe::rhi {

class Surface : Object<Surface> {
  friend struct SwapchainGenerationImpl;
  friend struct SwapchainImpl;

public:
  explicit Surface(void *handle) noexcept : Object(handle) {}
  Surface() noexcept : Object(nullptr) {}
  Surface(const Surface &) noexcept;
  Surface(Surface &&) noexcept;
  Surface &operator=(const Surface &) noexcept;
  Surface &operator=(Surface &&) noexcept;
  ~Surface() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  vulkan::Surface get() const noexcept;
  vulkan::Context *ctx() const noexcept;
  const Context &context() const noexcept;
};

} // namespace strobe::rhi
