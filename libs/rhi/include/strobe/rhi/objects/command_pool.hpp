#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"

namespace strobe::rhi {

class CommandPool {
  friend class Device;
  friend struct DeviceImpl;
  friend struct CommandBufferImpl;
  friend class CommandBuffer;
  friend struct CommandPoolImpl;

public:
  CommandPool() noexcept : m_handle(nullptr) {}
  CommandPool(const CommandPool &) noexcept;
  CommandPool(CommandPool &&) noexcept;
  CommandPool &operator=(const CommandPool &) noexcept;
  CommandPool &operator=(CommandPool &&) noexcept;
  ~CommandPool() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  CommandBuffer alloc(CommandBufferFlags flags = CommandBufferFlags::none);

private:
  CommandPool(void *handle) : m_handle(handle) {};
  void *m_handle;
};
} // namespace strobe::rhi
