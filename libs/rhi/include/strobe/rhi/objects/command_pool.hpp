#pragma once

#include "strobe/rhi/objects/command_buffer.hpp"
#include "strobe/rhi/types/command_buffer_type.hpp"

namespace strobe::rhi {

class CommandPool : Object<CommandPool> {
  friend class Device;
  friend struct DeviceImpl;
  friend struct CommandBufferImpl;
  friend class CommandBuffer;
  friend struct CommandPoolImpl;

public:
  CommandPool() noexcept : Object(nullptr) {}
  CommandPool(const CommandPool &) noexcept;
  CommandPool(CommandPool &&) noexcept;
  CommandPool &operator=(const CommandPool &) noexcept;
  CommandPool &operator=(CommandPool &&) noexcept;
  ~CommandPool() noexcept;
  explicit operator bool() const noexcept { return m_handle != nullptr; }

  CommandBuffer alloc(CommandBufferFlags flags = CommandBufferFlags::none);

  explicit CommandPool(void *handle) : Object(handle) {};
};
} // namespace strobe::rhi
