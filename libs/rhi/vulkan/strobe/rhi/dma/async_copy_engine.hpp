#pragma once

#include "strobe/rhi/dma/async_transfer_cmd.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/objects/timepoint.hpp"
#include "strobe/rhi/types/buffer_offset.hpp"
#include "strobe/rhi/types/image_layout.hpp"
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

  Timepoint async_copy(ImageRange dst, BufferImageRange src,
                       ImageLayout initialLayout,
                       ImageLayout finalLayout) noexcept;

  Timepoint async_copy(BufferImageRange dst, ImageRange src,
                       ImageLayout srcLayout) noexcept;

  Timepoint
  async_upload(BufferOffset dst, const void *src,
               uint64_t size = std::numeric_limits<uint64_t>::max()) noexcept;

  Timepoint
  async_upload(ImageRange dst, const void *src, uint32_t rowLength = 0,
               uint32_t imageHeight = 0,
               ImageLayout initialLayout = ImageLayout::transfer_dst,
               ImageLayout finalLayout = ImageLayout::transfer_dst) noexcept;

  AsyncTransferCmd async_cmd() noexcept;
};
} // namespace strobe::rhi
