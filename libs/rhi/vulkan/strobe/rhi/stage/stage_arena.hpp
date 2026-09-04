#pragma once

#include "strobe/rhi/buf/buffer_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/stage/staging_page.hpp"
#include "strobe/rhi/stage/staging_pool.hpp"
#include "strobe/rhi/stage/staging_pool_impl.hpp"
#include "strobe/rhi/vulkan/buffer.hpp"
#include "strobe/rhi/vulkan/cmd/transfer.hpp"

namespace strobe::rhi {

struct StageBuffer {
  vulkan::BufferOffset buffer;
  void *ptr;
};

// Access to the StageArena must be externally synchronized,
// but indirect access to the StagingBuffer is internally synchronized.
class StageArena {
public:
  explicit StageArena(StagingPool pool) noexcept : m_pool(std::move(pool)) {}
  StageArena(const StageArena &) = delete;
  StageArena(StageArena &&) = delete;
  StageArena &operator=(const StageArena &) = delete;
  StageArena &operator=(StageArena &&) = delete;
  ~StageArena() noexcept {
    auto *pool = object_handle_ptr<StagingPoolImpl>(m_pool);
    pool->recycle_page(m_page, m_tail);
  }

  StageBuffer alloc(VkDeviceSize size, uint16_t alignment = 1) {
    std::optional<StageBuffer> fastStage = try_alloc_from_page(size, alignment);
    if (fastStage) {
      return fastStage.value();
    }
    // Allocation failed => request new page
    auto *pool = object_handle_ptr<StagingPoolImpl>(m_pool);
    StagingPage *page = pool->alloc_page(size);
    // push front new page
    page->next = m_page;
    m_page = page;
    if (m_tail == nullptr) {
      m_tail = page;
    }
    m_top = 0; // reset arena (next alloc cannot fail)
    return *try_alloc_from_page(size, alignment);
  }

private:
  std::optional<StageBuffer> try_alloc_from_page(VkDeviceSize size,
                                                 uint16_t alignment) {
    if (m_page == nullptr) {
      return {};
    }
    assert(alignment != 0);
    assert(std::has_single_bit(alignment));

    VkDeviceSize offset = strobe::memory::align_up(m_top, alignment);
    if (offset > m_page->size() || size > m_page->size() - offset) {
      return {};
    }

    auto *base = static_cast<std::byte *>(m_page->buffer.ptr());
    vulkan::Buffer buffer =
        object_handle_ptr<BufferImpl>(m_page->buffer)->buffer;
    m_top = offset + size;
    return StageBuffer{{buffer, offset}, base + offset};
  }

  StagingPool m_pool;
  VkDeviceSize m_top = 0;
  StagingPage *m_page = nullptr;
  StagingPage *m_tail = nullptr;
};

} // namespace strobe::rhi
