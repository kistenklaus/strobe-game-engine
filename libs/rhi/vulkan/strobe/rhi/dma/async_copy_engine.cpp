#include "strobe/rhi/dma/async_copy_engine.hpp"
#include "strobe/rhi/dma/async_copy_engine_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include <limits>

namespace strobe::rhi {

void AsyncCopyEngine::pin(void *handle) noexcept {
  pin_void_handle<AsyncCopyEngineImpl>(handle);
}

void AsyncCopyEngine::unpin(void *handle) noexcept {
  unpin_void_handle<AsyncCopyEngineImpl>(handle);
}

Timepoint AsyncCopyEngine::async_copy(BufferOffset dst, BufferOffset src,
                                      uint64_t size) noexcept {
  auto *impl = void_handle_ptr<AsyncCopyEngineImpl>(m_handle);
  if (size == std::numeric_limits<uint64_t>::max()) {
    size = std::min(dst.buffer.size() - dst.offset,
                    src.buffer.size() - src.offset);
  }
  return impl->async_cmd().copy(dst, src, size).finish();
}
Timepoint AsyncCopyEngine::async_copy(ImageRange dst, BufferImageRange src,
                                      ImageLayout initialLayout,
                                      ImageLayout finalLayout) noexcept {
  auto *impl = void_handle_ptr<AsyncCopyEngineImpl>(m_handle);
  return impl->async_cmd().copy(dst, src, initialLayout, finalLayout).finish();
}

Timepoint AsyncCopyEngine::async_copy(BufferImageRange dst, ImageRange src,
                                      ImageLayout srcLayout) noexcept {
  auto *impl = void_handle_ptr<AsyncCopyEngineImpl>(m_handle);
  return impl->async_cmd().copy(dst, src, srcLayout).finish();
}

Timepoint AsyncCopyEngine::async_upload(BufferOffset dst, const void *src,
                                        uint64_t size) noexcept {

  auto *impl = void_handle_ptr<AsyncCopyEngineImpl>(m_handle);
  if (size == std::numeric_limits<uint64_t>::max()) {
    size = dst.buffer.size() - dst.offset;
  }
  return impl->async_cmd().upload(dst, src, size).finish();
}

Timepoint AsyncCopyEngine::async_upload(ImageRange dst, const void *src,
                                        uint32_t rowLength,
                                        uint32_t imageHeight,
                                        ImageLayout initialLayout,
                                        ImageLayout finalLayout) noexcept {
  auto *impl = void_handle_ptr<AsyncCopyEngineImpl>(m_handle);
  return impl->async_cmd()
      .upload(dst, src, rowLength, imageHeight, initialLayout, finalLayout)
      .finish();
}

AsyncTransferCmd AsyncCopyEngine::async_cmd() noexcept {
  auto *impl = void_handle_ptr<AsyncCopyEngineImpl>(m_handle);
  return impl->async_cmd();
}

} // namespace strobe::rhi
