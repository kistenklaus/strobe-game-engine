#pragma once

#include "strobe/rhi/dma/async_transfer_cmd.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"
#include <limits>
namespace strobe::rhi {

class AsyncCopyEngine : public Object<AsyncCopyEngine> {
  friend class Object<AsyncCopyEngine>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;
  explicit AsyncCopyEngine(void *handle) noexcept : Object(handle) {}

  Timepoint
  async_copy(BufferOffset dst, BufferOffset src,
             uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;
  Timepoint
  async_upload(BufferOffset dst, void *src,
               uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;

  AsyncTransferCmd async_cmd() noexcept;
};
} // namespace strobe::rhi
