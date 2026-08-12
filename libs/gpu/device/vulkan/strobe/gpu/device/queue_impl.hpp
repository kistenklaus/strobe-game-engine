#pragma once

#include "strobe/gpu/device/device.hpp"
#include "strobe/gpu/device/native_queue.hpp"
#include "strobe/gpu/device/queue_flags.hpp"
namespace strobe::gpu {

struct QueueImpl {
  QueueImpl(Device device, NativeQueue *native, QueueFlags flags = {}) noexcept
      : device(std::move(device)), native(native), flags(flags) {}
  QueueImpl(const QueueImpl &) = delete;
  QueueImpl(QueueImpl &&) = delete;
  QueueImpl &operator=(const QueueImpl &) = delete;
  QueueImpl &operator=(QueueImpl &&) = delete;

  ~QueueImpl() noexcept {
    native->release((flags & QueueFlags::exclusive) != 0);
  }

  Device device; // holds ref count.
  NativeQueue *const native;
  const QueueFlags flags;
};

} // namespace strobe::gpu
