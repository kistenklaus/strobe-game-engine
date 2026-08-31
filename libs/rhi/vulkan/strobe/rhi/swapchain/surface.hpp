#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/vulkan/surface.hpp"
namespace strobe::rhi {

class Device;

class Surface {
  friend class Device;
  friend struct SwapchainGenerationImpl;
  friend struct SwapchainImpl;

public:
  Surface() noexcept : m_handle(nullptr) {}

  Surface(const Surface &) noexcept;
  Surface(Surface &&) noexcept;

  Surface &operator=(const Surface &) noexcept;
  Surface &operator=(Surface &&) noexcept;

  ~Surface() noexcept;

  explicit operator bool() const noexcept { return m_handle != nullptr; }

  vulkan::Surface get() const noexcept;
  vulkan::Context *ctx() const noexcept;
  const Context& context() const noexcept;

private:
  explicit Surface(void *handle) noexcept : m_handle(handle) {}

  void *m_handle;
};

} // namespace strobe::rhi
