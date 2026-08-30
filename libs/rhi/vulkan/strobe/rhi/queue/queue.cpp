#include "strobe/rhi/queue/queue.hpp"
#include "strobe/rhi/queue/queue_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

Queue::Queue(const Queue & o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<QueueImpl>(m_handle);
  }
}

Queue::Queue(Queue && o) noexcept : Object(std::exchange(o.m_handle, nullptr)) {}

Queue &Queue::operator=(const Queue & o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle  != nullptr) {
    pin_void_handle<QueueImpl>(o.m_handle);
  }
  unpin_void_handle<QueueImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Queue &Queue::operator=(Queue && o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<QueueImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Queue::~Queue() noexcept { unpin_void_handle<QueueImpl>(m_handle); }

void Queue::wait(Timepoint timepoint, PipelineStage stage) noexcept {
  auto* impl = void_handle_ptr<QueueImpl>(m_handle);
  impl->wait(timepoint, stage);
}

void Queue::submit(span<const CommandBuffer> cmds) noexcept {
  auto* impl = void_handle_ptr<QueueImpl>(m_handle);
  impl->submit(cmds);
}

} // namespace strobe::rhi
