#include "strobe/rhi/dma/async_transfer_cmd.hpp"
#include "strobe/rhi/dma/async_copy_engine_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/vulkan/cmd/transfer.hpp"

namespace strobe::rhi {

AsyncTransferCmd &AsyncTransferCmd::copy(BufferOffset dst, BufferOffset src,
                                         uint64_t size) noexcept {
  m_impl->m_open.cmd.copy_buffer(dst, src, size);
  return *this;
}

AsyncTransferCmd &AsyncTransferCmd::copy(BufferOffset dst, StageBuffer src,
                                         uint64_t size) noexcept {
  auto *cmd_impl = object_handle_ptr<CommandBufferImpl>(m_impl->m_open.cmd);
  auto *dst_impl = object_handle_ptr<BufferImpl>(dst.buffer);
  vulkan::cmd_copy_buffer(cmd_impl->cmd,
                          {.buffer = dst_impl->buffer, .offset = dst.offset},
                          src.buffer, size);
  cmd_impl->state.retain(dst.buffer);
  return *this;
}
AsyncTransferCmd &AsyncTransferCmd::copy(ImageRange dst, BufferImageRange src,
                                         ImageLayout initialLayout,
                                         ImageLayout finalLayout) noexcept {
  if (initialLayout != ImageLayout::transfer_dst) {
    m_impl->m_open.cmd.transition_image(dst.image, initialLayout,
                                        ImageLayout::transfer_dst);
  }
  m_impl->m_open.cmd.copy_buffer_to_image(dst, src, ImageLayout::transfer_dst);
  if (finalLayout != ImageLayout::transfer_dst) {
    m_impl->m_open.cmd.transition_image(dst.image, ImageLayout::transfer_dst,
                                        finalLayout);
  }
  return *this;
}

AsyncTransferCmd &AsyncTransferCmd::copy(BufferImageRange dst, ImageRange src,
                                         ImageLayout srcLayout) noexcept {
  m_impl->m_open.cmd.copy_image_to_buffer(dst, src, srcLayout);
  return *this;
}

AsyncTransferCmd &AsyncTransferCmd::upload(BufferOffset dst, const void *src,
                                           uint64_t size) noexcept {
  m_impl->m_open.cmd.update(dst, src, size);
  return *this;
}
AsyncTransferCmd &AsyncTransferCmd::upload(ImageRange dst, const void *src,
                                           uint32_t rowLength,
                                           uint32_t imageHeight,
                                           ImageLayout initialLayout,
                                           ImageLayout finalLayout) noexcept {
  if (initialLayout != ImageLayout::transfer_dst) {
    m_impl->m_open.cmd.transition_image(dst.image, initialLayout,
                                        ImageLayout::transfer_dst);
  }
  m_impl->m_open.cmd.update(dst, src, rowLength, imageHeight,
                            ImageLayout::transfer_dst);
  if (finalLayout != ImageLayout::transfer_dst) {
    m_impl->m_open.cmd.transition_image(dst.image, ImageLayout::transfer_dst,
                                        finalLayout);
  }
  return *this;
}

StageBuffer AsyncTransferCmd::alloc_stage(uint64_t size,
                                          uint64_t alignment) noexcept {
  return object_handle_ptr<CommandBufferImpl>(m_impl->m_open.cmd)
      ->localStage.alloc(size, alignment);
}

Timepoint AsyncTransferCmd::finish() noexcept {
  Timepoint timepoint = m_impl->step_locked();
  m_lock.unlock();
  return timepoint;
}

AsyncTransferCmd::~AsyncTransferCmd() noexcept {
  if (m_lock.owns_lock()) {
    m_impl->step_locked();
  }
}

} // namespace strobe::rhi
