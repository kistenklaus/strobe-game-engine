#include "strobe/rhi/objects/queue.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/queue/queue_impl.hpp"

namespace strobe::rhi {

void Queue::pin(void *handle) noexcept { pin_void_handle<QueueImpl>(handle); }

void Queue::unpin(void *handle) noexcept {
  unpin_void_handle<QueueImpl>(handle);
}

void Queue::wait(const Timepoint &timepoint, PipelineStage stage) noexcept {
  if (!timepoint) {
    return;
  }
  ZoneScopedN("Queue::wait(Timepoint)");
  auto *impl = void_handle_ptr<QueueImpl>(m_handle);
  impl->wait(timepoint, stage);
}

void Queue::wait(const SwapchainImage &swapchainImage,
                 PipelineStage stage) noexcept {
  ZoneScopedN("Queue::wait(SwapchainImage)");
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

void Queue::flush() noexcept {
  auto *impl = void_handle_ptr<QueueImpl>(m_handle);
  ZoneScopedN("Queue::flush");
  impl->flush();
}

} // namespace strobe::rhi
