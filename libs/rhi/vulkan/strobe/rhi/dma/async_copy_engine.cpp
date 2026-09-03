#include "strobe/rhi/dma/async_copy_engine.hpp"
#include "strobe/rhi/dma/async_copy_engine_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include <limits>

namespace strobe::rhi {

AsyncCopyEngine::AsyncCopyEngine(const AsyncCopyEngine &o) noexcept
    : Object(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<AsyncCopyEngineImpl>(m_handle);
  }
}

AsyncCopyEngine::AsyncCopyEngine(AsyncCopyEngine &&o) noexcept
    : Object(std::exchange(o.m_handle, nullptr)) {}

AsyncCopyEngine &AsyncCopyEngine::operator=(const AsyncCopyEngine &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<AsyncCopyEngineImpl>(o.m_handle);
  }
  unpin_void_handle<AsyncCopyEngineImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

AsyncCopyEngine &AsyncCopyEngine::operator=(AsyncCopyEngine &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<AsyncCopyEngineImpl>(o.m_handle);
  }
  unpin_void_handle<AsyncCopyEngineImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

AsyncCopyEngine::~AsyncCopyEngine() noexcept {
  unpin_void_handle<AsyncCopyEngineImpl>(m_handle);
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

Timepoint AsyncCopyEngine::async_upload(BufferOffset dst, void *src,
                                        uint64_t size) noexcept {

  auto *impl = void_handle_ptr<AsyncCopyEngineImpl>(m_handle);
  if (size == std::numeric_limits<uint64_t>::max()) {
    size = dst.buffer.size() - dst.offset;
  }
  return impl->async_cmd().upload(dst, src, size).finish();
}

AsyncTransferCmd AsyncCopyEngine::async_cmd() noexcept {
  auto *impl = void_handle_ptr<AsyncCopyEngineImpl>(m_handle);
  return impl->async_cmd();
}

} // namespace strobe::rhi
