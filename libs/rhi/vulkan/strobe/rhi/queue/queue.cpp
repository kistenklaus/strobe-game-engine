#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/queue/queue_impl.hpp"

namespace strobe::rhi {

Queue::Queue(const Queue &o) noexcept : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<QueueImpl>(m_handle);
  }
}

Queue::Queue(Queue &&o) noexcept : Object(std::exchange(o.m_handle, nullptr)) {}

Queue &Queue::operator=(const Queue &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<QueueImpl>(o.m_handle);
  }
  unpin_void_handle<QueueImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

Queue &Queue::operator=(Queue &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<QueueImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Queue::~Queue() noexcept { unpin_void_handle<QueueImpl>(m_handle); }

void Queue::wait(const Timepoint &timepoint, PipelineStage stage) noexcept {
  auto *impl = void_handle_ptr<QueueImpl>(m_handle);
  impl->wait(timepoint, stage);
}

void Queue::wait(const SwapchainImage &swapchainImage,
                 PipelineStage stage) noexcept {
  ZoneScopedN("Queue::wait");
  auto *impl = void_handle_ptr<QueueImpl>(m_handle);
  impl->wait(swapchainImage, stage);
}

Timepoint Queue::submit(span<const CommandBuffer> cmds) noexcept {
  ZoneScopedN("Queue::submit");
  auto *impl = void_handle_ptr<QueueImpl>(m_handle);
  return impl->submit(cmds);
}

void Queue::present(SwapchainImage swapchainImage) noexcept {
  auto *impl = void_handle_ptr<QueueImpl>(m_handle);
  ZoneScopedN("Queue::present");
  impl->present(std::move(swapchainImage));
}

} // namespace strobe::rhi
