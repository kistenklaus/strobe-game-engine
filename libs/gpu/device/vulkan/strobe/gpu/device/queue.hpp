#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/gpu/device/binary_semaphore.hpp"
#include "strobe/gpu/device/fence.hpp"
#include "strobe/gpu/device/swapchain_image.hpp"
#include "strobe/gpu/device/timeline_semaphore.hpp"

namespace strobe::gpu {

class CommandBuffer;

struct TimelineSemaphoreSubmitInfo {
  TimelineSemaphore semaphore;
  uint64_t value = 0;
  // TODO: stages.
};

struct BinarySemaphoreSubmitInfo {
  BinarySemaphore semaphore;
  // TODO: stages.
};

struct SubmitInfo {
  span<CommandBuffer> cmds;
  span<BinarySemaphoreSubmitInfo> wait = {};
  span<BinarySemaphoreSubmitInfo> signal = {};
  Fence fence = {};
  span<TimelineSemaphoreSubmitInfo> signalTimeline = {};
  span<TimelineSemaphoreSubmitInfo> waitTimeline = {};
};

class Queue {
  friend class Device;
  friend class CommandBuffer;

public:
  Queue() : m_handle(nullptr) {}
  Queue(const Queue &) noexcept;
  Queue(Queue &&) noexcept;
  Queue &operator=(const Queue &) noexcept;
  Queue &operator=(Queue &&) noexcept;
  ~Queue() noexcept;

  void submit(const SubmitInfo &info);
  bool present(SwapchainImage &&image);

  explicit operator bool() const { return m_handle != nullptr; }

private:
  explicit Queue(void *handle) : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
