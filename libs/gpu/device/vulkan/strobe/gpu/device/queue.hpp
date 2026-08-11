#pragma once

#include "strobe/gpu/device/queue_flags.hpp"

namespace strobe::gpu {

class Queue {
  friend class DeviceImpl;

public:
  Queue(const Queue &);
  Queue(Queue &&) noexcept;
  Queue &operator=(const Queue &);
  Queue &operator=(Queue &&) noexcept;
  ~Queue() noexcept;

  void submit(/* ... */);
  void present(/* ... */);

private:
  explicit Queue(void *ptr) : m_handle(ptr) {}
  void *m_handle;
};

} // namespace strobe::gpu
