#pragma once

#include "strobe/gpu/device/command_buffer.hpp"
namespace strobe::gpu {

class CommandPool {
  friend class Device;
  friend struct DeviceImpl;
  friend struct CommandBufferImpl;

public:
  CommandPool() noexcept : m_handle(nullptr) {}
  CommandPool(const CommandPool &) noexcept;
  CommandPool(CommandPool &&) noexcept;
  CommandPool &operator=(const CommandPool &) noexcept;
  CommandPool &operator=(CommandPool &&) noexcept;
  ~CommandPool() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  CommandBuffer alloc(bool primary = true);
  void reset();

private:
  CommandPool(void *handle) : m_handle(handle) {};
  void *m_handle;
};
} // namespace strobe::gpu
