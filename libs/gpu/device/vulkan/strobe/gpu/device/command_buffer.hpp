#pragma once

#include "strobe/gpu/device/barrier.hpp"

namespace strobe::gpu {

class CommandBuffer {
  friend struct CommandPoolImpl;
  friend class CommandPool;
  friend struct SwapchainImpl;
  friend class Queue;

public:
  CommandBuffer() noexcept : m_handle(nullptr) {}
  CommandBuffer(const CommandBuffer &) noexcept;
  CommandBuffer(CommandBuffer &&) noexcept;
  CommandBuffer &operator=(const CommandBuffer &) noexcept;
  CommandBuffer &operator=(CommandBuffer &&) noexcept;
  ~CommandBuffer() noexcept;

  // pool must be externally synchronized
  void begin();
  // pool must be externally synchronized
  void end();

  void barrier(const Barrier &barrier);

private:
  CommandBuffer(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
