#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/queue/native_queue.hpp"
#include "strobe/rhi/types/queue_flags.hpp"

namespace strobe::rhi {

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

} // namespace strobe::rhi
