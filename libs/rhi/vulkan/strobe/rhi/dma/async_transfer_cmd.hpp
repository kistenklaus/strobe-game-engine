#pragma once

#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/stage/stage_arena.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"
#include <client/TracyLock.hpp>
#include <mutex>

namespace strobe::rhi {

struct AsyncCopyEngineImpl;

class AsyncTransferCmd {
#ifdef STROBE_RHI_TRACE_LOCKS
  using mutex = tracy::Lockable<std::mutex>;
#else
  using mutex = std::mutex;
#endif
  using unique_lock = std::unique_lock<mutex>;

public:
  AsyncTransferCmd &copy(BufferOffset dst, BufferOffset src,
                         uint64_t size) noexcept;

  AsyncTransferCmd &copy(BufferOffset dst, StageBuffer src, uint64_t size) noexcept;

  AsyncTransferCmd &upload(BufferOffset dst, const void *src,
                           uint64_t size) noexcept;

  StageBuffer alloc_stage(uint64_t size, uint64_t alignment) noexcept;

  Timepoint finish() noexcept;
  ~AsyncTransferCmd() noexcept;

private:
  friend struct AsyncCopyEngineImpl;
  explicit AsyncTransferCmd(unique_lock lock, AsyncCopyEngineImpl *impl)
      : m_lock(std::move(lock)), m_impl(impl) {}

  unique_lock m_lock;
  AsyncCopyEngineImpl
      *m_impl; // not a strong reference DMA must stay alive for lifetime.
};

} // namespace strobe::rhi
