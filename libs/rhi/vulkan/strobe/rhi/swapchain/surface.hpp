#pragma once

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

private:
  explicit Surface(void *handle) noexcept : m_handle(handle) {}

  void *m_handle;
};

} // namespace strobe::rhi
