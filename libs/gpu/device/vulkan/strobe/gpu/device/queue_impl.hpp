#pragma once

#include "strobe/gpu/device/native_queue.hpp"
#include "strobe/gpu/device/queue_flags.hpp"
namespace strobe::gpu {

struct QueueImpl {
  QueueImpl(NativeQueue *native, QueueFlags flags = {})
      : native(native), flags(flags) {}
  QueueImpl(const QueueImpl &) = delete;
  QueueImpl(QueueImpl &&) = delete;
  QueueImpl &operator=(const QueueImpl &) = delete;
  QueueImpl &operator=(QueueImpl &&) = delete;

  ~QueueImpl() noexcept {}

  NativeQueue *const native;
  const QueueFlags flags;
};

} // namespace strobe::gpu
